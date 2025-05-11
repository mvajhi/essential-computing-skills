#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define READ_DEVICE "/dev/lifo_read"
#define WRITE_DEVICE "/dev/lifo_write"
#define BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_written;
    pthread_t reader_thread;
    int option;
    int test_mode = 0;

    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }

    if (test_mode == 1) {
        // Test 1: Write data then read it
        printf("Test mode 1: Write then read\n");

        // Open write device
        printf("Opening %s\n", WRITE_DEVICE);
        fd = open(WRITE_DEVICE, O_WRONLY);
        if (fd < 0) {
            perror("Failed to open write device");
            return EXIT_FAILURE;
        }

        // Write data to device
        const char* test_data = "Hello LIFO driver!";
        printf("Writing: %s\n", test_data);
        bytes_written = write(fd, test_data, strlen(test_data));
        if (bytes_written < 0) {
            perror("Failed to write to device");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Wrote %zd bytes\n", bytes_written);
        close(fd);

        // Open read device
        printf("Opening %s\n", READ_DEVICE);
        fd = open(READ_DEVICE, O_RDONLY);
        if (fd < 0) {
            perror("Failed to open read device");
            return EXIT_FAILURE;
        }

        // Read data from device
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0) {
            perror("Failed to read from device");
            close(fd);
            return EXIT_FAILURE;
        }

        buffer[bytes_read] = '\0';
        printf("Read %zd bytes: %s\n", bytes_read, buffer);
        close(fd);

        // Read from empty buffer should return EOF (0 bytes)
        printf("Reading from empty buffer...\n");
        fd = open(READ_DEVICE, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            perror("Failed to open read device");
            return EXIT_FAILURE;
        }

        bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == 0) {
            printf("Correctly received EOF (0 bytes) from empty buffer\n");
        } else if (bytes_read < 0) {
            perror("Error reading from empty buffer");
        } else {
            printf("Unexpected: Read %zd bytes from supposedly empty buffer\n", bytes_read);
        }
        close(fd);

    } else if (test_mode == 2) {
        // Test 2: Demonstrate LIFO behavior with multiple writes and reads
        printf("Test mode 2: LIFO behavior demonstration\n");

        // Open write device
        printf("Opening %s\n", WRITE_DEVICE);
        fd = open(WRITE_DEVICE, O_WRONLY);
        if (fd < 0) {
            perror("Failed to open write device");
            return EXIT_FAILURE;
        }

        // Write multiple strings
        const char* test_strings[] = {"hi", "bye"};
        for (int i = 0; i < 2; i++) {
            printf("Writing: %s\n", test_strings[i]);
            bytes_written = write(fd, test_strings[i], strlen(test_strings[i]));
            if (bytes_written < 0) {
                perror("Failed to write to device");
                close(fd);
                return EXIT_FAILURE;
            }
            printf("Wrote %zd bytes\n", bytes_written);
        }
        close(fd);

        // Open read device
        printf("Opening %s\n", READ_DEVICE);
        fd = open(READ_DEVICE, O_RDONLY);
        if (fd < 0) {
            perror("Failed to open read device");
            return EXIT_FAILURE;
        }

        // Read back data - should come in LIFO order (last in, first out)
        printf("Reading from device (should be in LIFO order):\n");
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0)
        {
            perror("Failed to read from device");
            close(fd);
            return EXIT_FAILURE;
        }
        buffer[bytes_read] = '\0';
        printf("Read %zd bytes: %s\n", bytes_read, buffer);
        if (strcmp(buffer, "eybih") == 0)
        {
            printf("SUCCESS! Characters were correctly stored in LIFO order.\n");
        }
        else
        {
            printf("FAILURE! Expected \"eybih\" but got \"%s\"\n", buffer);
        }
        close(fd);
    } else {
        // Default: Interactive mode
        printf("LIFO Character Device Tester\n");
        printf("============================\n");
        printf("Select an option:\n");
        printf("1. Write data to LIFO\n");
        printf("2. Read data from LIFO\n");
        printf("3. Exit\n");

        while (1) {
            printf("\nEnter option (1-3): ");
            scanf("%d", &option);
            getchar();  // Consume newline

            switch (option) {
                case 1:
                    // Open write device
                    fd = open(WRITE_DEVICE, O_WRONLY);
                    if (fd < 0) {
                        perror("Failed to open write device");
                        break;
                    }

                    printf("Enter data to write: ");
                    fgets(buffer, BUFFER_SIZE, stdin);
                    buffer[strcspn(buffer, "\n")] = 0;  // Remove newline

                    bytes_written = write(fd, buffer, strlen(buffer));
                    if (bytes_written < 0) {
                        perror("Failed to write to device");
                    } else {
                        printf("Wrote %zd bytes\n", bytes_written);
                    }
                    close(fd);
                    break;

                case 2:
                    // Open read device
                    fd = open(READ_DEVICE, O_RDONLY);
                    if (fd < 0) {
                        perror("Failed to open read device");
                        break;
                    }

                    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
                    if (bytes_read < 0) {
                        perror("Failed to read from device");
                    } else if (bytes_read == 0) {
                        printf("LIFO is empty (EOF)\n");
                    } else {
                        buffer[bytes_read] = '\0';
                        printf("Read %zd bytes: %s\n", bytes_read, buffer);
                    }
                    close(fd);
                    break;

                case 3:
                    printf("Exiting...\n");
                    return EXIT_SUCCESS;

                default:
                    printf("Invalid option\n");
                    break;
            }
        }
    }

    return EXIT_SUCCESS;
}