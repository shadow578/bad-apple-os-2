/* -*- Mode: C; c-basic-offset: 3 -*- */

#include "types.h"
#include "console_vga.h"
#include "intr.h"
#include "datafile.h"

#include "screen.c"
#include "sound.c"
#include "cpuid.c"
#include "rtc.c"
#include "mem.c"

#define WAVE_SAMPLE_RATE 22050
DECLARE_DATAFILE(wave, chika_raw_z);
uint32 samplesLen;

int main(void)
{
   // initialize stuff
   ConsoleVGA_Init();
   Intr_Init();
   Intr_SetFaultHandlers(Console_UnhandledFault);
   Time_init();

   // load raw WAV file (8 bit unsigned mono at WAVE_SAMPLE_RATE Hz)
   samplesLen = DataFile_GetDecompressedSize(wave);
   uint8 *samples = malloc(samplesLen);
   DataFile_Decompress(wave, samples, samplesLen);
   Console_Format("Loaded %d samples\n", samplesLen);

   // test sound
   //Sound_playTone(2500);
   Time_delay(500);

   // start playback
   Console_WriteString("start");
   Sound_play(samples, samplesLen, WAVE_SAMPLE_RATE);

   // wait for playback to end
   uint32 currentSampleNo, lastSampleNo = 0;
   while (Sound_donePlaying() == FALSE)
   {
      currentSampleNo = Sound__waveCurrentSample;
      Console_Format("%d samples/s\n", currentSampleNo - lastSampleNo);
      lastSampleNo = currentSampleNo;
      Time_delay(1000);
   }
   Console_WriteString("\ndone.");
   Sound_stop();
   for (;;)
      ;
   return 0;
}
