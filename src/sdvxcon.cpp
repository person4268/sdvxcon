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
    int8_t x;
    int8_t y;
} __attribute__((packed));


union {
    struct {
        uint8_t buttons[12];
    } lights;
    uint8_t raw[12];
} light_data;

void hid_task();


void quadrature_testing_task() {
    gpio_init(8);
    gpio_set_pulls(8, true, false);

    int pos = 0;
    bool clicked = false;
    pio_add_program(pio0, &quadrature_encoder_program);
    // pins 4 and 5
    quadrature_encoder_program_init(pio0, 0, 4, 0);
    while(1) {
        int new_pos = quadrature_encoder_get_count(pio0, 0);
        int new_clicked = !gpio_get(8);

        if(new_pos != pos || new_clicked != clicked) {
            pos = new_pos;
            clicked = new_clicked;
            printf("Position: %d Clicked: %d\n", pos, clicked);
        }

    }
}

int main()
{
    board_init();
    tusb_init();
    stdio_init_all();
    set_sys_clock_hz(CUR_SYS_CLK, true);

    while(1) {
        hid_task();
        tud_task();
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
    report.x = board_millis() % 255;
    report.y = board_millis() % 255;

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
    if (report_id == 2 && report_type == HID_REPORT_TYPE_OUTPUT && buffer[0] == 2 && bufsize >= sizeof(light_data)) //light data
    {
        size_t i = 0;
        for (i; i < sizeof(light_data); i++)
        {
            light_data.raw[i] = buffer[i + 1];
        }
    }

    // echo back anything we received from host
    tud_hid_report(0, buffer, bufsize);
}