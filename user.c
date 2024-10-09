#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>

#define SHM_KEY 1234

// Shared memory structure for the clock
typedef struct {
    unsigned int seconds;
    unsigned int nanoseconds;
} shared_clock_t;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <duration>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned int duration = atoi(argv[1]);

    // Attach to the shared memory
    int shm_id = shmget(SHM_KEY, sizeof(shared_clock_t), 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shared_clock_t *shared_clock = (shared_clock_t *)shmat(shm_id, NULL, 0);
    if (shared_clock == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Get the current simulated time
    unsigned int start_seconds = shared_clock->seconds;
    unsigned int start_nanoseconds = shared_clock->nanoseconds;
    unsigned int end_nanoseconds = start_nanoseconds + duration;

    // Simulate the process execution
    while ((shared_clock->seconds < start_seconds) || 
           (shared_clock->seconds == start_seconds && shared_clock->nanoseconds < end_nanoseconds)) {
        // Busy wait, just checking the clock
    }

    // Print PID and clock details
    printf("Child process with PID %d terminating. Current time: %u:%u\n", getpid(), shared_clock->seconds, shared_clock->nanoseconds);

    // Detach from shared memory
    if (shmdt(shared_clock) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);

