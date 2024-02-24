#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <iostream>
#include <chrono>
#include <thread>



// Font set
unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

unsigned char memory[4096] = {0};

/*
Registers:
16 general purpose 8 bit registers, usually referred to as Vx
where x is a hexadecimal digit
a char is 8 bits, so we store 16 of them
*/
unsigned char V[16] = {0};

/*  
I register generally used to store memory addresses, so only the lowest rightmost 12 bits are usually used.
short is 16 bits
*/
unsigned short I = 0;

/*
PC register is the program counter is used to store the currently executing address
16 bits
*/
unsigned short PC = 0x200;

/*
the SP register is used to point to the topmost level of the stack
8 bit register
*/
unsigned char SP = 0;

/*
The stack:
is an array of 16 16 bit values, used to store the address that the interpreter
should return to when finished with a subroutine
*/
unsigned short Stack[16] = {0};

// Keyboard
unsigned char keyboard[16] = {0};

unsigned char display[64 * 32] = {0};

// Delay timer
unsigned char dt = 0;

// Sound timer
unsigned char st = 0;

// Display flag
unsigned char df = 0;
// Sound flag
unsigned char sf = 0;

void init_cpu(void) {
    // Seed the random number generator with the current time
    srand((unsigned int)time(NULL));

    // Load fonts into memory
    memcpy(memory, fontset, sizeof(fontset));
}

int load_rom(char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) return errno;

    struct stat st;
    stat(filename, &st);
    size_t fsize = st.st_size;

    size_t bytes_read = fread(memory + 0x200, 1, sizeof(memory) - 0x200, fp);
    fclose(fp);

    if (bytes_read != fsize) {
        return -1;
    }
    return 0;
}

void emulate_cycle(void) {
    
    df = 0;
    sf = 0;

    unsigned short op = memory[PC] << 8 | memory[PC + 1];
   
    unsigned short x = (op & 0x0F00) >> 8;
    unsigned short y = (op & 0x00F0) >> 4;

    switch (op & 0xF000) {
        case 0x0000:
            switch (op & 0x00FF) {
                case 0x00E0:
                    //std::cout << "[OK] 0x" << std::hex << op << ": 00E0" << std::endl;
                    for (int i = 0; i < 64 * 32; i++) {
                        display[i] = 0;
                    }
                    PC += 2;
                    break;
                case 0x00EE:
                    //std::cout << "[Ok] 0x" << std::hex << op << ": 00EE" << std::endl;
                    PC = Stack[SP];
                    SP--;
                    //double check this later
                    PC += 2;  // Make sure to increment PC here
                    break;
                default:
                    std::cout << "[Failed] Unknown Code" << std::hex << op << "0x" << std::endl;
                    break;
            }
            break;

        case 0x1000:
            //std::cout << "[OK] 1nnn" << std::hex << op << ": 1nnn" << std::endl;
            PC = op & 0xFFF;
            break;
        case 0x2000:
            SP++;
            Stack[SP] = PC;
            PC = op & 0xFFF;
            break;
        case 0x3000:
            if (V[x] == (op & 0x00FF)) {
                PC += 2;
            }
            PC += 2;
            break;
            
        case 0x4000:
            if (V[x] != (op & 0x00FF)) {
                PC += 2;
            }
            PC += 2;
            break;
          
        case 0x5000:
            if (V[x] == V[y]) {
                PC += 2;
            }
            PC += 2;
            break;
       
        case 0x6000:
            V[x] = (op & 0x00FF);
            PC += 2;
            break;
        case 0x7000:
            V[x] += (op & 0x00FF);
            PC += 2;
            break;
        case 0x8000:
            switch (op & 0x000F) {
                case 0x0000:
                    V[x] = V[y];
                    PC += 2;
                    break;
                case 0x0001:
                    V[x] = (V[x] | V[y]);
                    PC += 2;
                    break;
                case 0x0002:
                    V[x] = (V[x] & V[y]);
                    PC += 2;
                    break;
                case 0x0003:
                    V[x] = (V[x] ^ V[y]);
                    PC += 2;
                    break;
                case 0x0004:
                    V[x] = (V[x] + V[y] > 0xFF) ? 1 : 0;
                    V[x] += V[y];
                    PC += 2;
                    break;
                case 0x0005:
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] -= V[y];
                    PC += 2;
                    break;
                case 0x0006:
                    V[0xF] = V[x] & 0x1;
                    V[x] = (V[x] >> 1);
                    PC += 2;
                    break;
                case 0x0007:
                    V[0xF] = (V[y] > V[x]) ? 1 : 0;
                    V[x] = V[y] - V[x];
                    PC += 2;
                    break;
                case 0x000E:
                    V[0xF] = (V[x] >> 7) & 0x1;
                    V[x] = (V[x] << 1);
                    PC += 2;
                    break;
                default:
                    break;
            }
            
            break;
        case 0x9000:
            if (V[x] != V[y]) {
                PC += 2;
            }
            PC += 2;
            break;
        case 0xA000:
            I = op & 0x0FFF;
            PC += 2;
            break;
        case 0xB000:
            PC = (op & 0x0FFF) + V[0];
            break;
        case 0xC000:
            V[x] = (rand() % 256) & (op & 0x00FF);
            PC += 2;
            break;
        case 0xD000: {
            df = 1;

            unsigned short height = op & 0x000F;
            unsigned short px;

            // set collision flag to 0
            V[0xF] = 0;

            // loop over each row
            for (int yline = 0; yline < height; yline++) {
                // fetch the pixel value from the memory starting at location I
                px = memory[I + yline];

                // loop over 8 bits of one row
                for (int xline = 0; xline < 8; xline++) {
                    // check if the current evaluated pixel is set to 1 (0x80 >>
                    // xline scans through the byte, one bit at a time)
                    if ((px & (0x80 >> xline)) != 0) {
                        // if drawing causes any pixel to be erased, set the
                        // collision flag to 1
                        if (display[(V[x] + xline + ((V[y] + yline) * 64))] == 1) {
                            V[0xF] = 1;
                        }

                        // set pixel value by using XOR
                        display[V[x] + xline + ((V[y] + yline) * 64)] ^= 1;
                    }
                }
            }

            PC += 2;
            break;
        }
        case 0xE000:
            switch (op & 0x00FF) {
                case 0x009E:
                    if (keyboard[V[x]]) {
                        PC += 2;  // Increment by 4 to skip the next instruction
                    } 
                    PC += 2;
        
                    break;
                case 0x00A1:
                    if (!keyboard[V[x]]) {
                        PC += 2;  // Increment by 4 to skip the next instruction
                    } 
                    PC += 2;
                    
                    break;
                default:
                    printf("hi");
            }
            break;
        case 0xF000:
            switch (op & 0x00FF) {
                case 0x0007:
                    V[x] = dt;
                    PC += 2;
                    break;
                case 0x000A:
                    for (int i = 0; i < 16; i++) {
                        if (keyboard[i]) {
                            V[x] = i;
                            PC += 2;
                            break;
                        }
                    }
                    break;
                case 0x0015:
                    dt = V[x];
                    PC += 2;
                    break;
                case 0x0018:
                    st = V[x];
                    PC += 2;
                    break;
                case 0x001E:
                    I += V[x];
                    PC += 2;
                    break;
                case 0x0029:
                    I = V[x] * 5;
                    PC += 2;
                    break;
                case 0x0033:
                    memory[I] = (V[x] % 1000) / 100;
                    memory[I + 1] = (V[x] % 100) / 10;
                    memory[I + 2] = (V[x] % 10);
                    PC += 2;
                    break;
                case 0x0055:
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                    }
                    PC += 2;
                    break;
                case 0x0065:
                    for (int i = 0; i <= x; i++) {
                        V[i] = memory[I + i];
                    }
                    PC += 2;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if (dt > 0) dt -= 1;
    if (st > 0) {
        sf = 1;
        puts("BEEP");
        st -= 1;

    
    

    }
    

}
