#ifndef CHIP8_H_
#define CHIP8_H_

extern unsigned char fontset[80];
extern unsigned char memory[4096];
extern unsigned char V[16];
extern unsigned short I;
extern unsigned short PC;
extern unsigned char SP;
extern unsigned short Stack[16];
extern unsigned char keyboard[16];
extern unsigned char display[64 * 32];
extern unsigned char dt;
extern unsigned char st;
extern unsigned char df;
extern unsigned char sf;

void init_cpu(void);
int load_rom(char* filename);
void emulate_cycle(void);

#define error(...) fprintf(stderr, __VA_ARGS__) 

#endif