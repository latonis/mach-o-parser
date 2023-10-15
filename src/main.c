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

struct header_64 {
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
    void*   header;
    void**  load_commands;
};

int determine_type(struct macho_file *macho) {
    struct header* temp_header = (struct header*) macho->header;
    macho->is_64bit = 0;
    macho->should_swap = 0;
    macho->is_universal = 0;

    if (temp_header->magic == MH_CIGAM) {
        macho->should_swap = 1;
    } else if (temp_header->magic == MH_MAGIC_64) {
        macho->is_64bit = 1;
    } else if (temp_header->magic == MH_CIGAM_64) {
        macho->is_64bit = 1;
        macho->should_swap = 1;
    } else if (temp_header->magic == FAT_MAGIC) {
        macho->is_universal = 1;
    } else if (temp_header->magic == FAT_CIGAM) {
        macho->is_universal = 1;
        macho->should_swap = 1;
    }

    return 0;
}

uint8_t* get_header(uint8_t* input, struct macho_file *macho) {
    macho->header = (struct header_64*) input;
    return (uint8_t*) input + sizeof(struct header_64);
}


uint8_t* get_load_commands(uint8_t* input, struct macho_file* macho) {
    
    for (int i = 0; i < macho->header->n_commands; i++) {
        macho->load_commands[i] = (struct load_command*) input;
        input = (uint8_t*) (input + ((struct load_command*) macho->load_commands[i])->cmdsize);
    }

    return input;
}

int print_stats(struct macho_file* macho) {
    struct header* temp_header = (struct header*) macho->header;
    if (macho->is_64bit) {
        temp_header = 
    }
    printf("Magic: %x\n", macho->header->magic);
    printf("CPU Type: %x\n", macho->header->cpu_type);
    printf("CPU Subtype: %x\n", macho->header->cpu_subtype);
    printf("# of Commands: %x\n", macho->header->n_commands);
    printf("Size of Commands: %d\n", (int) macho->header->size_commands);
    printf("Swap: %u\n64 bit: %u\nFat: %u\n", macho->should_swap, macho->is_64bit, macho->is_universal);

    return 0;
}

int print_load_commands(struct macho_file* macho) {

    for (int i = 0; i < macho->header->n_commands; i++) {
        printf("cmd: %x\ncmdsize: %x\n", ((struct load_command*) macho->load_commands[i])->cmd, ((struct load_command*) macho->load_commands[i])->cmdsize); 
    }
    return 0;
}

uint8_t* read_file(uint8_t* input, FILE* fp) {
    if( !fp ) {
        perror("blah.txt");
        exit(1);
    }

    fseek( fp , 0L , SEEK_END);
    long lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    input = calloc(1, lSize + 1);
    if( !input ) {
        fclose(fp);
        fputs("memory alloc fails", stderr);
        exit(1);
    }

    /* copy the file into the buffer */
    if( fread( input , lSize, 1, fp) != 1) {
        fclose(fp);
        free(input);
        fputs("entire read fails",stderr);
        exit(1);
    }

    fclose(fp);

    return input;
}

int main(int argc, char **argv) {
    uint8_t* og_input;
    FILE* fp = fopen("/home/jacob/malware/macho/macho1", "rb");
    
    uint8_t* input = read_file(input, fp);

    struct macho_file* macho = malloc(sizeof(struct macho_file));
    macho->load_commands = malloc(sizeof(struct command *) * macho->header->n_commands);

    input = get_header(input, macho);
    determine_type(macho);

    input = get_load_commands(input, macho);

    print_stats(macho);
    print_load_commands(macho);

    free(macho);
    free(og_input);
}