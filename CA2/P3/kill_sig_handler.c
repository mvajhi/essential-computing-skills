#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handle_sigint(int sig) {
    printf("\nCtrl+C detected! (signal %d)\n", sig);
}

int main() {
    signal(SIGINT, handle_sigint); // Handle Ctrl+C

    while (1) {
        printf("Working... Press Ctrl+C to send SIGINT\n");
        sleep(2);
    }

    return 0;
}
