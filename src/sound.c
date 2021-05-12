/**
 * wip sound
 */
#include "types.h"
#include "io.h"
#include "intr.h"

#define PORT_CH0 0x40
//#define PORT_CH1 0x41 // not usable
#define PORT_CH2 0x42
#define PORT_PIT_CMD 0x43
#define PORT_KBD_SPEAKER 0x61

#define PIT_CH0 0b00000000
//#define PIT_CH1 0b01000000 // not usable
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

#define PIT_IRQ 0
#define PIT_BASE_FREQ 1193180
#define FREQ_TO_DIV(f) (PIT_BASE_FREQ / (f))

static inline void Sound__maybeEnableSpeaker()
{
    uint8 t = IO_In8(PORT_KBD_SPEAKER);
    if (t != (t | 3))
    {
        Console_WriteString("en_spk");
        IO_Out8(PORT_KBD_SPEAKER, t | 3);
    }
}

/**
 * play a tone at a specified frequency
 * @param f the frequency to play at
 */
static inline void Sound_playTone(uint16 f)
{
    uint16 d = FREQ_TO_DIV(f);
    IO_Out8(PORT_PIT_CMD, PIT_BINARY | PIT_MODE_3 | PIT_AM_LO_HI | PIT_CH2);
    IO_Out8(PORT_CH2, (uint8)(d & 0xFF));
    IO_Out8(PORT_CH2, (uint8)((d >> 8) & 0xFF));

    //enable speaker if needed
    Sound__maybeEnableSpeaker();
}

/**
 * stop all sound from playing
 */
static inline void Sound_stop()
{
    uint8 ctl = IO_In8(PORT_KBD_SPEAKER);
    IO_Out8(PORT_KBD_SPEAKER, ctl & 0xFC);
}

volatile uint8 *Sound__waveSamples;
volatile uint32 Sound__waveSamplesCount = 0;
volatile uint32 Sound__waveCurrentSample = 0;

/*
static void Sound__waveIRQ(void)
{
    // check if we are past the end
    //if (Sound__waveCurrentSample >= Sound__waveSamplesCount)
    //{
    //    return;
    //}

    // get the next sample
    //uint8 s = Sound__waveSamples[Sound__waveCurrentSample];

    // convert to the pit format
    //s = s >> 1;

    // program PIT
    //IO_Out8(PORT_PIT_CMD, 0xb0);
    //IO_Out8(PORT_CH2, s);
    //IO_Out8(PORT_CH2, 0);

    //Sound_playTone(s);

    asm("push %eax");

    // increment sample counter
    // Sound__waveCurrentSample++;

    // end isr
    IO_Out8(0x20, 0x20);
    asm("pop %eax\n"
        "iret");
}
*/

volatile void Sound__waveIRQ(int v)
{
    // check if we are past the end
    if (Sound__waveCurrentSample >= Sound__waveSamplesCount)
    {
        return;
    }

    Sound__waveCurrentSample++;

    // get the next sample
    uint8 s = Sound__waveSamples[Sound__waveCurrentSample];

    // convert to the pit format
    //s = s >> 1;

    // program PIT
    //IO_Out8(PORT_PIT_CMD, 0xb0);
    IO_Out8(PORT_CH2, s);
    // IO_Out8(PORT_CH2, 0);
}

/*
void Sound__waveIRQTwo(void);
asm(".global Sound__waveIRQTwo \n Sound__waveIRQTwo:\n"
    "pusha\n");
asm("call Sound__waveIRQ");
asm("movb $0x20, %al\n"
    "outb %al, $0x20\n"
    "popa\n"
    "iret");
*/

/**
 * play wave audio using the pc speaker in PWM mode.
 * uses PIT Channels 0 and 2, so you cannot use the PIT for anything other than this.
 *
 * @param samples the samples to play (8bit unsigned)
 * @param samplesCount how many samples to play
 * @param sampleRate sample rate to play at
 */
static inline void Sound_play(uint8 *samples, uint32 samplesCount, uint16 sampleRate)
{
    // disable interrupts
    // Intr_Disable();

    // set info for IRQ
    Sound__waveSamples = samples;
    Sound__waveSamplesCount = samplesCount;
    Sound__waveCurrentSample = 0;

    // register PIT Channel 0 IRQ handler
    // Intr_SetMask(PIT_IRQ, TRUE);
    Intr_SetHandler(IRQ_VECTOR(PIT_IRQ), Sound__waveIRQ);

    // directly set IQR0 handler, bypassing the "trampoline" and its extra overhead
    //Intr_SetDirect(IRQ_VECTOR(PIT_IRQ), Sound__waveIRQTwo);

    // unmask IRQ0
    Intr_SetMask(PIT_IRQ, TRUE);

    // setup PIT Channel 0 to fire periodically at the right rate
    uint32 ch0Div = FREQ_TO_DIV(sampleRate);
    IO_Out8(PORT_PIT_CMD, PIT_BINARY | PIT_MODE_2 | PIT_AM_LO_HI | PIT_CH0);
    IO_Out8(PORT_CH0, (uint8)(ch0Div & 0xFF));
    IO_Out8(PORT_CH0, (uint8)((ch0Div >> 8) & 0xFF));

    Console_Format("SR: %d; DIV: %d\n", sampleRate, ch0Div);

    // setup PIT Channel 2
    IO_Out8(PORT_PIT_CMD, PIT_BINARY | PIT_MODE_0 | PIT_AM_HI_ONLY | PIT_CH2);
    IO_Out8(PORT_CH2, 0);
    IO_Out8(PORT_PIT_CMD, PIT_BINARY | PIT_MODE_0 | PIT_AM_LO_ONLY | PIT_CH2);
    IO_Out8(PORT_CH2, 0);

    // enable speaker
    Sound__maybeEnableSpeaker();

    // enable interrupts
    Intr_Enable();
}

/**
 * check if the current wave sound is done playing
 * @returns are we done playing?
 */
static inline Bool Sound_donePlaying()
{
    if (Sound__waveCurrentSample >= Sound__waveSamplesCount)
        return TRUE;
    else
        return FALSE;
}
