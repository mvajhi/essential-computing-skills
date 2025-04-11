#include <stdio.h>
#include <stdlib.h>

#define MEGABYTE (1024 * 1024)
#define MEMORY_SIZE (500 * MEGABYTE)

int main() {
    // Allocate 500 MB of memory
    char *memory_block = (char *)malloc(MEMORY_SIZE);
    
    if (memory_block == NULL) {
        printf("Error: Memory allocation failed!\n");
        return 1;
    }
    
    printf("Successfully allocated 500 MB of memory.\n");
    
    // Use the memory (e.g., fill it with zeros)
    printf("Filling the allocated memory...\n");
    for (size_t i = 0; i < MEMORY_SIZE; i++) {
        memory_block[i] = 0;
    }
    printf("Memory filled successfully.\n");
    
    while (1);

    // Free the memory
    free(memory_block);
    printf("Memory freed.\n");
    
    return 0;
}