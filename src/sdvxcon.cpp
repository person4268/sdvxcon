#include <cmath>
#include <cstdio>
#include <cstring>

#include "pico/stdlib.h"

#include "ws2812.h"
#include "quadrature.pio.h"
#include "consts.h"

#include "bsp/board_api.h"
#include "tusb.h"


struct report {
    uint16_t buttons;
    int16_t x;
    int16_t y;
    int16_t rx;
    int16_t ry;
} __attribute__((packed));


union {
    struct {
        uint8_t buttons[12];
    } lights;
    uint8_t raw[12];
} light_data;


void hid_task();


void quad_init() {
    gpio_init(8);
    gpio_set_pulls(8, true, false);
    pio_add_program(pio0, &quadrature_encoder_program);
    // pins 4 and 5
    quadrature_encoder_program_init(pio0, 0, 4, 0);
    // pins 2 and 3
    quadrature_encoder_program_init(pio0, 1, 2, 0);

}

int32_t quad2_get_pos() {
    return quadrature_encoder_get_count(pio0, 0);
}

int32_t quad1_get_pos() {
    return quadrature_encoder_get_count(pio0, 1);
}

static WS2812 ws2812;

int main()
{
    board_init();
    tusb_init();
    stdio_init_all();
    set_sys_clock_hz(CUR_SYS_CLK, true);
    quad_init();
    ws2812.init();

    while(1) {
        hid_task();
        tud_task();
        ws2812.runws2812();
    }
}


void hid_task() {
    // Poll every 1ms
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time

    start_ms += interval_ms;

    // Remote wakeup
    if (tud_suspended()) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }

    // Update button status
    struct report report;
    report.buttons = ((board_millis() / 1000) % 2) << ((board_millis() / 2000) % 12);
    report.x = quad2_get_pos() * (65536.0f/80.0f);
    report.y = 0;
    report.rx = 0;
    report.ry = 0;

    // Send the 3 bytes HID report
    if (tud_hid_ready())
        tud_hid_report(1, &report, sizeof(report));
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
    if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT && bufsize >= sizeof(light_data)) //light data
    {
        size_t i = 0;
        for (i; i < sizeof(light_data); i++)
        {
            light_data.raw[i] = buffer[i];
            if(i == 0) {
                ws2812.set_color(buffer[i], buffer[i], buffer[i]);
            }
        }
    }

    // echo back anything we received from host
    tud_hid_report(0, buffer, bufsize);
}

// Invoked when device is mounted
void tud_mount_cb(void) {}

// Invoked when device is unmounted
void tud_umount_cb(void) {}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
}
