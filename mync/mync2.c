#include <stdio.h>
#include <stdlib.h>     // For functions like mkstemp
#include <unistd.h>     // For system functions like fork, dup2, execlp
#include <sys/wait.h>   // For functions like waitpid
#include <string.h>     // For functions like strcmp

void mync_program(const char *program) {
    // Fork the process
    pid_t pid = fork(); // create a new process (child process) that is a copy of the parent process.
    if (pid == -1) { // If fork fails
        printf("Error occurred in fork");
        return;
    }

    if (pid == 0) {  // Child process
        // Execute the given program:
        // /bin/sh - filename or path of the program to be executed.
        // sh - name of the program (in our case is the shell).
        // -c - an argument to the shell sh. It tells the shell to read commands from the next argument, which is program.
        // program - arguments to the program.
        // (char *) NULL - The list of arguments must be terminated by a NULL pointer.
        execlp("/bin/sh", "sh", "-c", program, (char *) NULL);
        // the existing code and memory of the current process are replaced by the new program (/bin/sh in this case).
        // The process ID remains the same, but its memory and program are entirely replaced.
        // if the command completes or fails, the process ends there.
        // If there is any error in executing the shell or the command, execlp returns -1.
        printf("Error occurred in execlp");
        return;
    } else {  // Parent process
        // Wait for the child process to finish
        // pid: The process ID of the child process to wait for: 
        // 1. > 0: Wait for the child process with the specific process ID
        // 2. -1: will wait for the first child process that changes state (usually terminate), regardless of which child it is.
        // 3. 0: Wait for any child process in the same process group as the caller.
        // *status: A pointer to an integer where the information about how the child process terminated. This information includes whether the child process exited normally, was killed by a signal, or stopped.
        // options: Flags that affect the behavior of waitpid. For our purposes, we use 0, meaning no special options are set.
        int status;
        waitpid(pid, &status, 0);

        // WIFEXITED(status) checks if the child process terminated normally.
        // If it did, WEXITSTATUS(status) is used to get the exit status of the child process (the value passed to exit or returned from main).
        if (WIFEXITED(status)) {
            printf("Program exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Program did not exit normally\n");
        }
    }
}

/*
The mync_program function:
1. Forks a new process. In the child process, it executes the specified program using execlp.
2. The parent process waits for the child process to finish and prints the exit status.
*/

int main(int argc, char *argv[]) {
    if (argc != 3 || strcmp(argv[1], "-e") != 0) { // The "-e" flag is used to clearly indicate that the following argument is the program to execute.
        fprintf(stderr, "Usage: %s -e <program>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *program = argv[2];
    mync_program(program);

    return 0;
}

/* HOW TO RUN:

1)  ./mync2 -e "./ttt 123456789" -i TCPS4050
    nc localhost 4050
*/

/* Creating gcov:

1)  make
2) running the program
3) gcov -b mync2.c
*/