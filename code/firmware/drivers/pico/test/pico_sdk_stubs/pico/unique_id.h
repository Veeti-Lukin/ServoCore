#ifndef PICO_UNIQUE_ID_H
#define PICO_UNIQUE_ID_H
// HOST TEST STUB
typedef unsigned int uint;
#define PICO_UNIQUE_BOARD_ID_SIZE_BYTES 8
void pico_get_unique_board_id_string(char* id_out, uint len);
#endif
