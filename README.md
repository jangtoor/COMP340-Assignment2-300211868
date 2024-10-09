# COMP340-Assignment2-300211868

Overview of the Project
This project serves as an example of concurrent UNIX processes with shared memory. A shared memory-based simulated clock is used by the main process (oss) to control the creation, execution, and termination of child processes (user). Every child process keeps an eye on the simulated clock and runs for a predetermined amount of time. The parent process limits the number of processes running concurrently, logs events to an output file, and maintains tabs on the child processes.

oss.c: the main program that

Sets aside shared memory to run a clock simulation.
uses exec() and fork() to create child processes up to a given limit.
keeps track of and tracks the child processes' lifespan.
controls signal handling for shared memory cleanup and termination.
user.c: The application for the child process that

Attaches to the shared memory and reads the simulated clock.
waits for the timer to go off in order for its term to end.
records the specifics of its termination.
Makefile: Compiles user.c and oss.c automatically.

input.txt: Contains timing information for when child processes should be launched and their durations.

output.txt: Records actions like the start and stop of child processes as well as the simulated clock values.

.gitignore: Makes sure that version control does not track transitory or superfluous files (such as executables and object files).

Compilation Instructions
To compile the project, run the following command in the project directory:
make
This will create two executables:

oss: The parent process manager.
user: The child process program that is executed by oss.

Running the Project
To run oss, use the following command with the appropriate options:
./oss [-n x] [-s y] [-i inputfile] [-o outputfile] [-h]

Options:
-n x: Specifies the maximum number of child processes to create (default: 4).
-s y: Specifies the maximum number of concurrent child processes (default: 2).
-i inputfile: Specifies the input file containing the clock increment and child process timing data (default: input.txt).
-o outputfile: Specifies the output file to log the events (default: output.txt).
-h: Displays help information and the default values for all options.

Input File Format
The input.txt file should follow this format:
<increment_value>
<seconds> <nanoseconds> <duration>
<seconds> <nanoseconds> <duration>
...

increment_value: The amount of nanoseconds that will be added to the clock for each iteration of the loop.
seconds and nanoseconds: The earliest time when a child process can be launched.
length: The amount of time (measured in nanoseconds) that the child process will operate.

Example of an input.txt file:
20000
0 64867 375891
0 563620 5000000
0 2758375 375620
1 30000 35000

Program Execution and Output
When OSS executes, it

processes the options from the command line.
gives the clock a shared memory allocation.
retrieves the launch durations and timings of child processes from the input.txt file.
generates child processes and reports events in output.txt at the designated simulated time.
keeps an eye on child processes to make sure that no more than -s concurrent processes are active at once.
terminates (using a timer signal) after two actual seconds or when all child processes have finished.
Every young person processes:

reads the simulated clock by attaching itself to the shared memory.
waits the time that has been allotted.
logs its PID and termination time before closing.

Example log entry in output.txt:
Launched child process with PID 1234 at simulated time 0:64867 with duration 5000000
Child process 1234 terminated at simulated time 1:305867

Signal Handling SIGALRM: After two actual seconds, the program ends automatically. Upon receiving this signal, shared memory is cleared, the clock's final state is logged, and all child processes that are currently operating are terminated.

SIGINT (Ctrl+C): If the user actively interrupts the program, it will log the last clock values, clean up shared memory, kill any child processes that are currently executing, and terminate properly.

Testing
To test the program, you can run oss with various values for -n and -s options. For example:
./oss -n 10 -s 3 -i input.txt -o output.txt

This rule will:

Ten child processes at most should be created.
Permit up to three child processes to operate in parallel.
For the clock increments and process durations, use input.txt.
Record the happenings in output.txt.
You may watch how the application manages the development and termination of child processes and simulate situations where processes go above their boundaries. To verify how the application releases shared memory and ends, use signals like Ctrl+C.

Problems Addressed
Shared memory attachment/detachment: At first, I had problems with user.c's shared memory detachment because the child process wasn't detached correctly. By carefully controlling shmdt() and making sure it is always called before a child process quits, I was able to resolve issue.

Signal Handling: It was difficult to make sure that shared memory was released and that every process was properly ended. Using full signal handling for SIGALRM and SIGINT was the solution.

Version control was implemented in this project through the use of Git. An example of a commit is shown below:
commit 1a2b3c4: Implemented shared memory setup and signal handling
commit 5d6e7f8: Added child process management and fork-exec logic
commit 9g0h1i2: Finished input parsing and added logging to output.txt

The .gitignore file includes entries to ignore executables and object files, ensuring the repository only contains necessary source files.
