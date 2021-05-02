#include "types.h"
#include "vbe.h"
#include "console_vga.h"

#define SCREEN_W 800
#define SCREEN_H 600
#define BPP 32
typedef uint32 color_t;

color_t _backBuffer[SCREEN_W * SCREEN_H];

static inline color_t rgb(uint8 r, uint8 g, uint8 b)
{
    return (r << 16) | (g << 8) | b;
}

static inline void printVBEInfo()
{
    Console_WriteString("init VESA...\n\n");
    if (!VBE_Init())
    {
        Console_Panic("VBE init failed. :(");
    }

    // print vbe details
    Console_Format("Found VBE %x.%02x\n"
                   "\n"
                   " OEM: '%s'\n"
                   " Vendor: '%s'\n"
                   " Product: '%s'\n"
                   " Revision: '%s' \n",
                   gVBE.cInfo.verMajor,
                   gVBE.cInfo.verMinor,
                   PTR_FAR_TO_32(gVBE.cInfo.oemString),
                   PTR_FAR_TO_32(gVBE.cInfo.vendorName),
                   PTR_FAR_TO_32(gVBE.cInfo.productName),
                   PTR_FAR_TO_32(gVBE.cInfo.productRev));
    Console_Flush();
}

static inline void initScreen()
{
    VBE_InitSimple(SCREEN_W, SCREEN_H, BPP);
}

static inline void setPixel(uint16 x, uint16 y, color_t c)
{
    _backBuffer[(y * SCREEN_W) + x] = c;
}

static inline void swapScreen()
{
    memcpy32(gVBE.current.info.linearAddress, _backBuffer, sizeof(_backBuffer) / 4);
}

static inline void clearScreen(color_t c)
{
    memset32(_backBuffer, c, sizeof(_backBuffer) / 4);
}