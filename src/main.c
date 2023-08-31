#include "main.h"
#include <string.h>

struct macho_file {
    // https://stackoverflow.com/questions/44579663/mach-o-magic-and-cigam-clarification
    uint8_t is_64bit;
    uint8_t is_universal;
    uint8_t should_swap;
    uint32_t magic;
    int position;
    int file_size;
    FILE* stream;

};

int get_int(struct macho_file *macho, uint8_t* buffer) {
    int magic_len = fread(buffer, sizeof(int), 1, macho->stream);

    return 0;
}

int determine_type(struct macho_file *macho) {

    if (macho->magic == MH_CIGAM) {
        macho->should_swap = 1;
    } else if (macho->magic == MH_MAGIC_64) {
        macho->is_64bit = 1;
    } else if (macho->magic == MH_CIGAM_64) {
        macho->is_64bit = 1;
        macho->should_swap = 1;
    } else if (macho->magic == FAT_MAGIC) {
        macho->is_universal = 1;
    } else if (macho->magic == FAT_CIGAM) {
        macho->is_universal = 1;
        macho->should_swap = 1;
    }

    return 0;
}

int open_file(struct macho_file *macho) {
    macho->stream = fopen("../examples/macho2", "rb");

    fseek(macho->stream, 0L, SEEK_END);    // seek to the EOF
    macho->file_size = ftell(macho->stream);       // get the current position
    rewind(macho->stream);

    return 0;
}

int main(int argc, char **argv) {

    struct macho_file *macho = malloc(sizeof(struct macho_file));
    
    open_file(macho);
    uint8_t* buffer = malloc(sizeof(uint8_t)*4);

    get_int(macho, buffer);
    memcpy(&macho->magic, buffer, sizeof(uint32_t));
    // rewind(stream);

    for (int i = 0; i < 4; i++) {
        printf("Data read from file: %x \n", buffer[i]);
    }

    determine_type(macho);
    printf("%x\n", macho->magic);

    printf("Swap? %d\n64 bit? %d\nFat? %d\n", macho->should_swap, macho->is_64bit, macho->is_universal);
    printf("File Size: %d\n", macho->file_size);
    fclose(macho->stream);

    return 0;
}