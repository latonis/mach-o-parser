#include "main.h"
#include <string.h>

struct header {
    uint32_t magic;
    cpu_type_t cpu_type;
    cpu_subtype_t cpu_subtype;
    uint32_t filetype;
    uint32_t n_commands;
    uint32_t size_commands;
    uint32_t flags;
};

struct macho_file {
    // https://stackoverflow.com/questions/44579663/mach-o-magic-and-cigam-clarification
    uint8_t is_64bit;
    uint8_t is_universal;
    uint8_t should_swap;
    struct header* header;
    int position;
    int file_size;
    FILE* stream;
};

int get_int(struct macho_file *macho, uint8_t* buffer) {
    return fread(buffer, sizeof(int), 1, macho->stream);
}

int determine_type(struct macho_file *macho) {

    if (macho->header->magic == MH_CIGAM) {
        macho->should_swap = 1;
    } else if (macho->header->magic == MH_MAGIC_64) {
        macho->is_64bit = 1;
    } else if (macho->header->magic == MH_CIGAM_64) {
        macho->is_64bit = 1;
        macho->should_swap = 1;
    } else if (macho->header->magic == FAT_MAGIC) {
        macho->is_universal = 1;
    } else if (macho->header->magic == FAT_CIGAM) {
        macho->is_universal = 1;
        macho->should_swap = 1;
    }

    return 0;
}

int open_file(struct macho_file *macho) {
    macho->stream = fopen("../examples/macho1", "rb");

    fseek(macho->stream, 0L, SEEK_END);    // seek to the EOF
    macho->file_size = ftell(macho->stream);       // get the current position
    rewind(macho->stream);

    return 0;
}

int main(int argc, char **argv) {
    // welcome to the danger zone :-)

    // initialize things
    struct macho_file *macho = malloc(sizeof(struct macho_file));
    macho->header = malloc(sizeof(struct header));
    
    open_file(macho);
    uint8_t* buffer = malloc(sizeof(uint8_t)*4);

    // get magic bytes
    get_int(macho, buffer);
    memcpy(&macho->header->magic, buffer, sizeof(uint32_t));

    free(buffer);

    // cpu type
    buffer = malloc(sizeof(cpu_type_t));
    get_int(macho, buffer);
    memcpy(&macho->header->cpu_type, buffer, sizeof(cpu_type_t));
    free(buffer);

    buffer = malloc(sizeof(cpu_subtype_t));
    get_int(macho, buffer);
    memcpy(&macho->header->cpu_subtype, buffer, sizeof(cpu_subtype_t));
    free(buffer);

    for (int i = 0; i < 4; i++) {
        printf("Data read from file: %x \n", buffer[i]);
    }

    determine_type(macho);
    printf("%x\n", macho->header->magic);
    printf("%x\n", macho->header->cpu_type);
    printf("%x\n", macho->header->cpu_subtype);


    printf("Swap? %d\n64 bit? %d\nFat? %d\n", macho->should_swap, macho->is_64bit, macho->is_universal);
    printf("File Size: %d\n", macho->file_size);
    fclose(macho->stream);


    return 0;
}