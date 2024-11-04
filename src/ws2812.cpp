#include "ws2812.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "bsp/board_api.h"
#include "ws2812.pio.h"
#include "consts.h"
#include <cmath>

// order is 8 on the bottom front left 8, side left 7, back left 11, back right 11, side right 5, front right 8
#define STRIP_LENGTH (8 + 7 + 11 + 11 + 5 + 8)

uint8_t buf[STRIP_LENGTH * 3] = {0};

WS2812::WS2812() {}

void WS2812::init() {
    PIO pio = WS2812_PIO;
    uint sm = 1;
    uint offset = pio_add_program(pio, &ws2812_program);
    uint pin = 22;
    ws2812_program_init(pio, sm, offset, pin, 800000, false);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(WS2812_PIO, 1, pixel_grb << 8u);
}

void WS2812::runws2812() {
    if(board_millis() % 5 == 0) {
        for (int i = 0; i < STRIP_LENGTH; i++) {
            uint32_t r = buf[i * 3];
            uint32_t g = buf[i * 3 + 1];
            uint32_t b = buf[i * 3 + 2];

            put_pixel(urgb_u32(r, g, b));
        }
    }
}

/* 0 == r, 1 == g, 2 == b */
static inline void set_buf_range(uint8_t start, uint8_t end, uint16_t r, uint16_t g, uint16_t b) {
    r = pow(r / 255.0, 2.2) * 255;
    g = pow(g / 255.0, 2.2) * 255;
    b = pow(b / 255.0, 2.2) * 255;

    // r *= (255 / 255);
    g *= (224.0/255.0);
    b *= (140.0/255.0);

    for (int i = start; i < end; i++) {
        buf[i * 3 + 0] = r;
        buf[i * 3 + 1] = g;
        buf[i * 3 + 2] = b;
    }
}

void WS2812::set_bfl(uint8_t r, uint8_t g, uint8_t b) {
    set_buf_range(0, 0 + 8, r, g, b);
}

void WS2812::set_sl(uint8_t r, uint8_t g, uint8_t b) {
    set_buf_range(0 + 8, 0 + 8 + 7, r, g, b);
}

void WS2812::set_bl(uint8_t r, uint8_t g, uint8_t b) {
    set_buf_range(0 + 8 + 7, 0 + 8 + 7 + 11, r, g, b);
}

void WS2812::set_br(uint8_t r, uint8_t g, uint8_t b) {
    set_buf_range(0 + 8 + 7 + 11, 0 + 8 + 7 + 11 + 11, r, g, b);
}

void WS2812::set_sr(uint8_t r, uint8_t g, uint8_t b) {
    set_buf_range(0 + 8 + 7 + 11 + 11, 0 + 8 + 7 + 11 + 11 + 5, r, g, b);
}

void WS2812::set_bfr(uint8_t r, uint8_t g, uint8_t b) {
    set_buf_range(0 + 8 + 7 + 11 + 11 + 5, 0 + 8 + 7 + 11 + 11 + 5 + 8, r, g, b);
}

uint32_t WS2812::hsv_to_urgb(uint32_t hue, float s, float v) {
    float c = s * v;
    float x = c * (1 - fabsf(fmodf(hue / 60.0f, 2) - 1));
    float m = v - c;
    
    float r, g, b;
    
    if (hue >= 0 && hue < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (hue >= 60 && hue < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (hue >= 120 && hue < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (hue >= 180 && hue < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (hue >= 240 && hue < 300) {
        r = x;
        g = 0;
        b = c;
    } else {
        r = c;
        g = 0;
        b = x;
    }
    
    uint8_t red = (uint8_t)((r + m) * 255);
    uint8_t green = (uint8_t)((g + m) * 176);
    uint8_t blue = (uint8_t)((b + m) * 240);
    
    return urgb_u32(red, green, blue);
}

uint32_t WS2812::urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

uint32_t WS2812::urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            ((uint32_t) (w) << 24) |
            (uint32_t) (b);
}
