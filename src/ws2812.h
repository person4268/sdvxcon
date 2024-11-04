#pragma once
#include <cstdint>

class WS2812 {
  public:
    WS2812();
    void init();
    void runws2812();
    uint32_t hsv_to_urgb(uint32_t hue, float s, float v);
    uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
    uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void set_bfl(uint8_t r, uint8_t g, uint8_t b);
    void set_sl(uint8_t r, uint8_t g, uint8_t b);
    void set_bl(uint8_t r, uint8_t g, uint8_t b);
    void set_br(uint8_t r, uint8_t g, uint8_t b);
    void set_sr(uint8_t r, uint8_t g, uint8_t b);
    void set_bfr(uint8_t r, uint8_t g, uint8_t b);
  protected:
  private:
};;