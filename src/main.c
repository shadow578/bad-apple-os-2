/* -*- Mode: C; c-basic-offset: 3 -*- */

#include "types.h"
#include "console_vga.h"
#include "intr.h"

#include "screen.c"
#include "timing.c"
#include "sound.c"
#include "cpuid.c"
#include "rtc.c"

volatile uint32 counter = 0;

void rtcIrq(int v)
{
   counter++;
   RTC_ackInterrupt();
}

int main(void)
{
   // initialize stuff
   ConsoleVGA_Init();
   Intr_Init();
   Intr_SetFaultHandlers(Console_UnhandledFault);
   initTiming();

   // TODO rtc testing

   // irq
   RTC_registerIRQ(rtcIrq);

   // clock
   rtcTime_t time;
   for (;;)
   {
      RTC_readRTC(&time);
      Console_MoveTo(0, 0);
      Console_Format("%d-%d-%d  %d:%d:%d  -- %d",
                     time.year, time.month, time.dayOfMonth,
                     time.hour, time.minute, time.second,
                     counter);

      wait(500);
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
   wait(10000);
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
