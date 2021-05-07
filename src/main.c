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

DECLARE_DATAFILE(data, test_raw_z);
uint32 len;

int main(void)
{
   // initialize stuff
   ConsoleVGA_Init();
   Intr_Init();
   Intr_SetFaultHandlers(Console_UnhandledFault);
   Time_init();

   // datafile testing
   len = DataFile_GetDecompressedSize(data);

   // malloc testing
   char *data_buffer = malloc(len + 1);

   DataFile_Decompress(data, data_buffer, 1024);
   data_buffer[len] = '\0';

   Console_Flush();
   Console_WriteString(data_buffer);
   Console_Flush();

   // clock
   rtcTime_t time;
   for (;;)
   {
      RTC_readRTC(&time);
      Console_MoveTo(0, 0);
      Console_Format("%d-%d-%d  %d:%d:%d  -- %d",
                     time.year, time.month, time.dayOfMonth,
                     time.hour, time.minute, time.second,
                     Time_now());

      Time_delay(500);
   }

   // vbe info
   printVBEInfo();

   // cpu info
   char vendor[17];
   CPUID_getCpuIdVendorString(vendor);
   Console_Format("CPU Vendor: %s", vendor);

   // pc speaker sound
   //playTone(1193180 / 800);

   // timing
   Time_delay(10000);
   Console_WriteString("wait_end");

   /*
   for (;;)
   {
      Console_MoveTo(0, 0);
      Console_Format("T=%d", now());
   }
*/

   // vesa graphics
   initScreen();
   clearScreen(rgb(255, 0, 0));
   swapScreen();
   for (uint16 x = 0; x < SCREEN_W; x++)
      for (uint16 y = 0; y < SCREEN_H; y++)
         if (x == y)
            setPixel(x, y, rgb(0, 255, 0));

   swapScreen();

   return 0;
}
