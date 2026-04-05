#include "vga.h"
#include "io.h"

static const uint8_t g_13h_misc = 0x63;
static const uint8_t g_13h_seq[5] = {0x03,0x01,0x0F,0x00,0x0E};
static const uint8_t g_13h_crtc[25] = {
    0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,
    0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x9C,
    0x0E,0x8F,0x28,0x40,0x96,0xB9,0xA3,0xFF,0x00
};
static const uint8_t g_13h_graphics[9] = {0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF};
static const uint8_t g_13h_attr[21] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x41,0x00,0x0F,0x00,0x00
};

static void vga_write_regs(void) {
    outb(0x3C2, g_13h_misc);

    /* Sequencer */
    for (int i = 0; i < 5; ++i) {
        outb(0x3C4, i);
        outb(0x3C5, g_13h_seq[i]);
    }

    /* CRTC */
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & ~0x80);

    for (int i = 0; i < 25; ++i) {
        outb(0x3D4, i);
        outb(0x3D5, g_13h_crtc[i]);
    }

    /* Graphics controller */
    for (int i = 0; i < 9; ++i) {
        outb(0x3CE, i);
        outb(0x3CF, g_13h_graphics[i]);
    }

    /* Attribute controller */
    for (int i = 0; i < 21; ++i) {
        (void)inb(0x3DA);  /* reset flip-flop */
        outb(0x3C0, i);
        outb(0x3C0, g_13h_attr[i]);
    }

    /* Finally enable graphics mode */
    (void)inb(0x3DA);
    outb(0x3C0, 0x20);
}

static void vga_set_palette_256(void) {
    /* Set up a reasonable 256-color VGA palette (xterm-256 compatible cube). */
    outb(0x3C8, 0x00);
    for (int i = 0; i < 256; i++) {
        uint8_t r, g, b;
        if (i < 16) {
            /* Basic EGA/RGB 4-bit table (shades simplified). */
            static const uint8_t ega_colors[16][3] = {
                {0x00,0x00,0x00},{0x00,0x00,0xAA},{0x00,0xAA,0x00},{0x00,0xAA,0xAA},
                {0xAA,0x00,0x00},{0xAA,0x00,0xAA},{0xAA,0x55,0x00},{0xAA,0xAA,0xAA},
                {0x55,0x55,0x55},{0x55,0x55,0xFF},{0x55,0xFF,0x55},{0x55,0xFF,0xFF},
                {0xFF,0x55,0x55},{0xFF,0x55,0xFF},{0xFF,0xFF,0x55},{0xFF,0xFF,0xFF}
            };
            r = ega_colors[i][0];
            g = ega_colors[i][1];
            b = ega_colors[i][2];
        } else if (i < 232) {
            int idx = i - 16;
            int ri = idx / 36;
            int gi = (idx / 6) % 6;
            int bi = idx % 6;
            r = (ri * 255) / 5;
            g = (gi * 255) / 5;
            b = (bi * 255) / 5;
        } else {
            int gray = 8 + (i - 232) * 10;
            r = g = b = (uint8_t)gray;
        }
        /* Convert 0-255 into 0-63 DAC values */
        outb(0x3C9, (uint8_t)(r * 63 / 255));
        outb(0x3C9, (uint8_t)(g * 63 / 255));
        outb(0x3C9, (uint8_t)(b * 63 / 255));
    }
}

int vga_set_mode_13h(void) {
    vga_write_regs();
    vga_set_palette_256();
    /* Not much checking possible, just assume success */
    return 0;
}

void vga13_clear(uint8_t color) {
    uint8_t *fb = (uint8_t *)0xA0000;
    for (int i = 0; i < 320 * 200; ++i) {
        fb[i] = color;
    }
}

void vga13_putpixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 320 || y < 0 || y >= 200) return;
    uint8_t *fb = (uint8_t *)0xA0000;
    fb[y * 320 + x] = color;
}
