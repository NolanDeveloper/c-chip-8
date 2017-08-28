#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

extern int get_pixel(int row, int col);
extern void reset_ram();
extern int load_file(char * filename);
extern int disassemble(char * filename);
extern void set_key_state(unsigned n, unsigned state);
extern void tick();
