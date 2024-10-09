#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#define MAX_PROCESSES 20

// Shared memory structure to store the simulated clock
typedef struct {
    unsigned int seconds;
    unsigned int nanoseconds;
} shared_clock_t;

// Shared memory variables
key_t shm_key = 1234;
int shm_id;
shared_clock_t *shared_clock = NULL;

// Function declarations
void increment_clock(unsigned int increment);
void handle_sigint(int sig);
void handle_sigalrm(int sig);
void cleanup();

int main(int argc, char *argv[]) {
    int max_processes = 4, concurrent_processes = 2;
    char *input_file = "input.txt", *output_file = "output.txt";
    unsigned int clock_increment;
    
    // Parse command-line arguments
    int opt;
    while ((opt = getopt(argc, argv, "n:s:i:o:h")) != -1) {
        switch (opt) {
            case 'n': max_processes = atoi(optarg); break;
            case 's': concurrent_processes = atoi(optarg); break;
            case 'i': input_file = optarg; break;
            case 'o': output_file = optarg; break;
            case 'h':
                printf("Usage: %s [-n max_processes] [-s concurrent_processes] [-i inputfile] [-o outputfile]\n", argv[0]);
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s [-n max_processes] [-s concurrent_processes] [-i inputfile] [-o outputfile]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (max_processes > MAX_PROCESSES) {
        fprintf(stderr, "Error: Max processes cannot exceed %d\n", MAX_PROCESSES);
        exit(EXIT_FAILURE);
    }

    // Set up signal handlers
    signal(SIGINT, handle_sigint);  // For Ctrl+C
    signal(SIGALRM, handle_sigalrm); 
    alarm(2);  // Set alarm for 2 seconds to auto-terminate

    // Allocate shared memory
    shm_id = shmget(shm_key, sizeof(shared_clock_t), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shared_clock = (shared_clock_t*)shmat(shm_id, NULL, 0);
    if (shared_clock == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Initialize shared clock
    shared_clock->seconds = 0;
    shared_clock->nanoseconds = 0;

    // Open input file
    FILE *input_fp = fopen(input_file, "r");
    if (!input_fp) {
        perror("Error opening input file");
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Read the first line for clock increment value
    if (fscanf(input_fp, "%u", &clock_increment) != 1) {
        perror("Error reading clock increment from input file");
        fclose(input_fp);
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Open output file
    FILE *output_fp = fopen(output_file, "w");
    if (!output_fp) {
        perror("Error opening output file");
        fclose(input_fp);
        cleanup();
        exit(EXIT_FAILURE);
    }

    // Main loop for process management
    int active_processes = 0, total_processes = 0;
    pid_t pids[MAX_PROCESSES];
    unsigned int launch_seconds, launch_nanoseconds, duration;

    while (fscanf(input_fp, "%u %u %u", &launch_seconds, &launch_nanoseconds, &duration) == 3) {
        while (active_processes >= concurrent_processes) {
            // Wait for any child to finish
            pid_t pid = wait(NULL);
            if (pid > 0) {
                fprintf(output_fp, "Child process %d terminated at simulated time %u:%u\n", pid, shared_clock->seconds, shared_clock->nanoseconds);
                active_processes--;
            }
        }

        // Wait for the clock to reach the launch time
        while (shared_clock->seconds < launch_seconds || (shared_clock->seconds == launch_seconds && shared_clock->nanoseconds < launch_nanoseconds)) {
            increment_clock(clock_increment);
        }

        // Fork a new process
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            cleanup();
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Child process
            char duration_str[20];
            snprintf(duration_str, 20, "%u", duration);
            execl("./user", "user", duration_str, NULL);
            perror("execl failed");
            exit(EXIT_FAILURE);
        } else {  // Parent process
            pids[total_processes] = pid;
            fprintf(output_fp, "Launched child process with PID %d at simulated time %u:%u with duration %u\n", pid, shared_clock->seconds, shared_clock->nanoseconds, duration);
            active_processes++;
            total_processes++;
        }

        if (total_processes >= max_processes) {
            break;
        }
    }

    // Wait for all remaining child processes to terminate
    while (active_processes > 0) {
        pid_t pid = wait(NULL);
        if (pid > 0) {
            fprintf(output_fp, "Child process %d terminated at simulated time %u:%u\n", pid, shared_clock->seconds, shared_clock->nanoseconds);
            active_processes--;
        }
    }

    // Cleanup and final clock output
    fprintf(output_fp, "Final simulated clock time: %u:%u\n", shared_clock->seconds, shared_clock->nanoseconds);
    fclose(input_fp);
    fclose(output_fp);
    cleanup();
    return 0;
}

// Function to increment the clock
void increment_clock(unsigned int increment) {
    shared_clock->nanoseconds += increment;
    if (shared_clock->nanoseconds >= 1000000000) {
        shared_clock->seconds += 1;
        shared_clock->nanoseconds -= 1000000000;
    }
}

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("SIGINT received. Cleaning up and terminating...\n");
    cleanup();
    exit(EXIT_SUCCESS);
}

// Signal handler for SIGALRM (2-second timeout)
void handle_sigalrm(int sig) {
    printf("SIGALRM received. Timeout. Cleaning up and terminating...\n");
    cleanup();
    exit(EXIT_SUCCESS);
}

// Cleanup function to detach and remove shared memory
void cleanup() {
    if (shmdt(shared_clock) == -1) {
        perror("shmdt failed");
    }
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    }
}

