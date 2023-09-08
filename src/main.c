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
    uint32_t reserved;
};


struct macho_file {
    // https://stackoverflow.com/questions/44579663/mach-o-magic-and-cigam-clarification
    uint8_t is_64bit;
    uint8_t is_universal;
    uint8_t should_swap;
    struct header* header;
    struct load_command** load_commands;
    int position;
    int file_size;
    FILE* stream;
};

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

int get_uint32(struct macho_file *macho, uint32_t* buffer) {
    return fread(buffer, sizeof(uint32_t), 1, macho->stream);
}


int get_header(struct macho_file *macho) {
    fread(macho->header, sizeof(struct header)- sizeof(uint32_t) , 1, macho->stream);
    determine_type(macho);

    if (macho->is_64bit) {
        get_uint32(macho, &macho->header->reserved);
        // fread(&macho->header->reserved, sizeof(uint32_t), 1, macho->stream);
    }

}

int get_commands(struct macho_file *macho) {

    for (int i = 0; i < macho->header->n_commands; i++) {
        macho->load_commands[i] = malloc(sizeof(struct load_command));
        get_uint32(macho, &macho->load_commands[i]->cmd);
        get_uint32(macho, &macho->load_commands[i]->cmdsize);
        fseek(macho->stream, macho->load_commands[i]->cmdsize-(sizeof(uint32_t) * 2), SEEK_CUR);
    }
}


int open_file(struct macho_file *macho) {
    macho->stream = fopen("/home/jacob/malware/macho/macho1", "rb");

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
    get_header(macho);
    macho->load_commands = (struct load_command**) malloc(sizeof(struct command *) * macho->header->n_commands);
    get_commands(macho);

    printf("Magic: %x\n", macho->header->magic);
    printf("CPU Type: %x\n", macho->header->cpu_type);
    printf("CPU Subtype: %x\n", macho->header->cpu_subtype);
    printf("# of Commands: %x\n", macho->header->n_commands);
    printf("Size of Commands: %d\n", (int) macho->header->size_commands);

    for (int i = 0; i < macho->header->n_commands; i++) {
        printf("Command: %x\n", macho->load_commands[i]->cmd);
        printf("Command Size: %x\n", macho->load_commands[i]->cmdsize);
    }

    printf("Swap: %d\n64 bit: %d\nFat: %d\n", macho->should_swap, macho->is_64bit, macho->is_universal);
    printf("File Size: %d\n", macho->file_size);
    fclose(macho->stream);


    return 0;
}