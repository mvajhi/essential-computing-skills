#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void signal_handler(int sig) {
    printf("\nSignal %d received!\n", sig);
}

int main() {
    for (int i = 1; i < NSIG; i++) { 
        if (signal(i, signal_handler) == SIG_ERR) {
            printf("Cannot handle signal %d\n", i);
        }
    }

    while (1) {
        printf("Waiting for signals...\n");
        sleep(3);
    }

    return 0;
}
