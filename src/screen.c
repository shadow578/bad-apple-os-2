/**
 * provides a wrapper for the vbe.h VESA functions
 */
#include "types.h"
#include "vbe.h"
#include "console_vga.h"

#define SCREEN_W 800
#define SCREEN_H 600
#define BPP 32
typedef uint32 color_t;

color_t VESA__backBuffer[SCREEN_W * SCREEN_H];

/**
 * create a new 32- bit rgb- value
 * @param r red part
 * @param g green part
 * @param b blue part
 * @returns 32 bit color
 */
static inline color_t rgb(uint8 r, uint8 g, uint8 b)
{
    return (r << 16) | (g << 8) | b;
}

/**
 * print vesa implementation and vendor info to console
 */
static inline void VESA_printInfo()
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

/**
 * initialize the screen
 */
static inline void VESA_init()
{
    VBE_Init();
    VBE_InitSimple(SCREEN_W, SCREEN_H, BPP);
    memset(VESA__backBuffer, 0, sizeof(VESA__backBuffer));
}

/**
 * write a pixel to the screen back buffer
 * @param x the x position of the pixel
 * @param y the y position of the pixel
 * @param c the color of the pixel
 */
static inline void VESA_write(uint16 x, uint16 y, color_t c)
{
    VESA__backBuffer[(y * SCREEN_W) + x] = c;
}

/**
 * swap the back buffer and the front buffer.
 * call this when you're finished drawing to the screen
 */
static inline void VESA_swap()
{
    memcpy32(gVBE.current.info.linearAddress, VESA__backBuffer, sizeof(VESA__backBuffer) / 4);
}

/**
 * clear the screen with a color
 * @param c the color to clear with
 */
static inline void VESA_clear(color_t c)
{
    memset32(VESA__backBuffer, c, sizeof(VESA__backBuffer) / 4);
}