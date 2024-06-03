#include <stdlib.h>
#include <string.h>

static char *output_file = NULL;

char *get_output_file(void) { return output_file; }

int main(int argc, char **argv) {
    if (argc == 3) {
        strcpy(output_file, argv[2]);
    }
    return EXIT_SUCCESS;
}
