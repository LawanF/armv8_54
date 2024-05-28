#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/*
    Takes a FILE *fileptr and a long int *size.
    Stores the size of file (in bytes) in size.
*/
void filesize(FILE *fileptr, long int *size) {
    if (fseek(fileptr, 0, SEEK_END)) {
        fprintf(stderr, "filesize: fseek error, errno %d\n", errno);
        exit(1);
    }

    if ((*size = ftell(fileptr)) < 0) {
        fprintf(stderr, "filesize: ftell error, errno %d\n", errno);
        exit(1);
    }

    if (fseek(fileptr, 0, SEEK_SET)) {
        fprintf(stderr, "filesize: fseek error, errno  %d\n", errno);
        exit(1);
    }
}

/*
    Takes a string that specifies a file location.
    Returns a char array with the contents of that file.
*/
char *store_file_to_arr(char *filename) {
    // Define array to return.
    char *arr;

    // Open file and check if it opens successfully.
    FILE *fileptr = fopen(filename, "r");
    if (fileptr == NULL) {
        fprintf(stderr, "store_file_to_arr: can’t open %s, errno %d\n", filename, errno);
        exit(1);
    }

    // Store filesize in size.
    long int size;
    filesize(fileptr, &size);

    // Allocate memory to arr and check if allocation is successful.
    arr = malloc(size);
    if (arr == NULL) {
        fprintf(stderr, "store_file_to_arr: ran out of memory, errno %d\n", errno);
        exit(1);
    }

    // Read file into arr.
    fread(arr, 1, size, fileptr);

    // Close file.
    fclose(fileptr);

    return arr;
}
