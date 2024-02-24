#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "inc/chip8.h"
#include "inc/peripherals.h"

extern int should_quit;

int main(int argc, char** argv) {
    char rom_filename[256]; // Assuming a reasonable maximum size for the filename

    if (argc == 2) {
        // If a filename is provided as a command-line argument, use it
        strncpy(rom_filename, argv[1], sizeof(rom_filename));
    } else {
        // If no filename provided, prompt the user to enter it
        printf("Enter the ROM file name: ");
        if (scanf("%255s", rom_filename) != 1) {
            fprintf(stderr, "[ERROR] Failed to read ROM file name.\n");
            return 1;
        }
    }

    printf("[INFO] Initializing CHIP-8 architecture...\n");
    init_cpu();
    puts("[OK] Initialization completed successfully!");

    printf("[INFO] Loading ROM %s...\n", rom_filename);

    int error = load_rom(rom_filename);
    if (error) {
        if (error == -1) {
            fprintf(stderr, "[ERROR] fread() failure: the return value was not equal to the ROM file size.\n");
        } else {
            perror("[ERROR] Error while loading ROM");
        }
        return 1;
    }

    puts("[OK] ROM loaded successfully!");

    init_display();
    puts("[OK] Display successfully initialized.");

    while (!should_quit) {
        emulate_cycle();
        sdl_ehandler(keyboard);

        if (df) {
            draw(display);
        }

        // Delay to emulate CHIP-8's clock speed.
        usleep(1500);
    }

    stop_display();
    return 0;
}
