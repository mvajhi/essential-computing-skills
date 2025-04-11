#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#define NUM_THREADS 8  

void* cpu_intensive_task(void* arg) {
    while (1) {
        for (long i = 0; i < 1000000; i++) {
            double val = sin(i) * cos(i) * tan(i);
            val = sqrt(val * val);
        }
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    
    printf("Starting CPU stress test with %d threads...\n", NUM_THREADS);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, cpu_intensive_task, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }
    
    printf("CPU is now under heavy load. Press Ctrl+C to stop.\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}