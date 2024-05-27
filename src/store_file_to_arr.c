#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

char *store_file_to_arr(char *filename) {
    // Define array to return.
    char *arr;

    // Open file and check if it opens successfully.
    FILE *fileptr = fopen(filename, "r");
    if (fileptr == NULL) {
        fprintf(stderr, "store_file_to_arr: can’t open %s\n", filename);
        exit(1);
    }

    // Get its length in bytes and return pointer to start of file.
    fseek(fileptr, 0, SEEK_END);
    long length = ftell(fileptr);
    fseek(fileptr, 0, SEEK_SET);

    // Allocate memory to arr and check if allocation is successful.
    arr = malloc(length);
    if (arr == NULL) {
        fprintf(stderr, "store_file_to_arr: ran out of memory.");
        exit(1);
    }

    // Read file into arr.
    fread(arr, 1, length, fileptr);

    // Close file.
    fclose(fileptr);

    return arr;
}
