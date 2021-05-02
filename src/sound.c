#include "types.h"
#include "io.h"

#define PORT_CH2 0x42
#define PORT_PIT_CMD 0x43
#define PORT_ 0x61

#define PIT_CH0 0b00000000
#define PIT_CH1 0b01000000
#define PIT_CH2 0b10000000
#define PIT_AM_LATCH 0b00000000
#define PIT_AM_LO_ONLY 0b00010000
#define PIT_AM_HI_ONLY 0b00100000
#define PIT_AM_LO_HI 0b00110000
#define PIT_MODE_0 0b00000000
#define PIT_MODE_1 0b00000010
#define PIT_MODE_2 0b00000100
#define PIT_MODE_3 0b00000110
#define PIT_MODE_4 0b00001000
#define PIT_MODE_5 0b00001010
//#define PIT_MODE_2 0b00001100
//#define PIT_MODE_3 0b00001110
#define PIT_BINARY 0b00000000
#define PIT_BCD 0b00000000

static inline void playSound(uint16 d)
{
    IO_Out8(PORT_PIT_CMD, PIT_BINARY | PIT_MODE_3 | PIT_AM_LO_HI | PIT_CH2);
    IO_Out8(PORT_CH2, (uint8)(d & 0xFF));
    IO_Out8(PORT_CH2, (uint8)((d >> 8) & 0xFF));

    uint8 ctl = IO_In8(0x61);
    if (ctl != (ctl | 3))
    {
        IO_Out8(0x61, ctl | 3);
    }
}

static inline void stopSound()
{
    uint8 ctl = IO_In8(0x61);
    IO_Out8(0x61, ctl & 0xFC);
}