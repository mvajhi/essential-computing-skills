#include <stdio.h>
#include <signal.h>
#include <unistd.h>

// Signal handler function
void signal_handler(int sig) {
    printf("\nUnhandled signal %d received.\n", sig);
}

int main() {
    struct sigaction sa;

    // Set the signal handler
    sa.sa_handler = signal_handler;
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls
    sigemptyset(&sa.sa_mask); // No additional signals to block

    // Register signals
    sigaction(SIGINT, &sa, NULL);  // Ctrl+C
    sigaction(SIGTSTP, &sa, NULL); // Ctrl+Z
    sigaction(SIGQUIT, &sa, NULL); // Ctrl+\

    // Infinite loop to keep the program running
    while (1) {
        printf("Waiting for signals... Press Ctrl+C, Ctrl+Z, or Ctrl+\\ to test.\n");
        sleep(2);
    }

    return 0;
}
