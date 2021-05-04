/* -*- Mode: C; c-basic-offset: 3 -*- */

#include "types.h"
#include "console_vga.h"
#include "intr.h"

#include "screen.c"
#include "timing.c"
#include "sound.c"
#include "cpuid.c"

int main(void)
{
   // initialize stuff
   ConsoleVGA_Init();
   Intr_Init();
   Intr_SetFaultHandlers(Console_UnhandledFault);
   initTiming();

   // vbe info
   printVBEInfo();

   // cpu info
   char vendor[17];
   getCpuIdVendorString(vendor);
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
