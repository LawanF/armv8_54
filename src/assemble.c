#include <stdlib.h>

static char *input_file = NULL;
static char *output_file = NULL;

int main(int argc, char **argv) {

    if (!(argc == 3)) {
        fprintf(stderr, "usage: ./assemble [input_file] [output_file]");
        exit(1);
    }

    output_file = argv[2];
    input_file = argv[1];

    // initialise if there is anything?

    // loop read each line until EOF. parse each line, pass to encode, pass to binary file writer.
    // where does two pass come in
    char buffer[20];
    while ( fgets(buffer, sizeof(buffer), in) != NULL) {
        parse_ins

    return EXIT_SUCCESS;
}
