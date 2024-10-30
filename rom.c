#include <stdio.h>

typedef struct {
    char entry_point[4];
    char logo[0x30];
    char title[16];
    char new_licensee_code[2];
    char sgb_flag;
    char cart_type;
    char rom_size;
    char ram_size;
    char dest_code;
    char old_licensee_code;
    char rom_ver_number;
    char header_checksum;
    char global_checksum[2];
} cart_header;

int main(int argc, char **argv) {
    FILE *rom = fopen(argv[1], "rb");
    if (rom == NULL) {
        perror("Error opening ROM file");
        return 1;
    }

    if (fseek(rom, 0x100, SEEK_SET) != 0) {
        perror("Error seeking to header");
        fclose(rom);
        return 1;
    }

    // Create an instance of the cart_header struct
    cart_header header;

    // Read the header data into the struct
    if (fread(&header, sizeof(cart_header), 1, rom) != 1) {
        perror("Error reading header");
        fclose(rom);
        return 1;
    }

    // Close file after reading
    fclose(rom);

    // Access the header data
    printf("Title: %.16s\n", header.title);
    printf("ROM Size: %d\n", header.rom_size);
    printf("Cartridge Type: %d\n", header.cart_type);
    printf("New licensee code: %.2s\n", header.new_licensee_code);

    return 0;
}
