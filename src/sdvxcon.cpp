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
        uint8_t strip[18];
    } lights;
    uint8_t raw[18];
} light_data;


void hid_task();
void set_leds_task();


void quad_init() {
    // gpio_init(8);
    // gpio_set_pulls(8, true, false);
    pio_add_program(pio0, &quadrature_encoder_program);
    pio_add_program(pio1, &quadrature_encoder_program);
    // pins 4 and 5
    quadrature_encoder_program_init(pio1, 0, 4, 0);
    // pins 2 and 3
    quadrature_encoder_program_init(pio0, 0, 2, 0);

}

int32_t quad2_get_pos() {
    return quadrature_encoder_get_count(pio1, 0);
}

int32_t quad1_get_pos() {
    return quadrature_encoder_get_count(pio0, 0);
}

static WS2812 ws2812;

union btn_states {
    struct button_states {
        uint16_t fx_l : 1; // 18
        uint16_t fx_r : 1; // 15
        uint16_t btn_a : 1; // 17
        uint16_t btn_b : 1; // 16
        uint16_t btn_c : 1; // 13
        uint16_t btn_d : 1; // 14
        uint16_t aux_1 : 1; // 19
        uint16_t aux_2 : 1; // 20
        uint16_t aux_3 : 1; // 21
        uint16_t aux_4 : 1; // 12
        uint16_t aux_5 : 1; // 11
        uint16_t aux_6 : 1; // 10
        uint16_t padding : 4;
    } __attribute__((packed)) states;
    uint16_t raw;
};
// all active low
static int btns[] = {18, 15, 17, 16, 13, 14, 19, 20, 21, 10, 11, 12};

int main()
{
    board_init();
    tusb_init();
    stdio_init_all();
    set_sys_clock_hz(CUR_SYS_CLK, true);
    quad_init();
    ws2812.init();

    for (int i = 0; i < sizeof(btns) / sizeof(int); i++)
    {
        gpio_init(btns[i]);
        gpio_set_dir(btns[i], GPIO_IN);
        gpio_pull_up(btns[i]);
    }

    while(1) {
        hid_task();
        tud_task();
        set_leds_task();
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
    // report.buttons = ((board_millis() / 1000) % 2) << ((board_millis() / 2000) % 12);
    btn_states states = {};
    states.states.fx_l = !gpio_get(18);
    states.states.fx_r = !gpio_get(15);
    states.states.btn_a = !gpio_get(17);
    states.states.btn_b = !gpio_get(16);
    states.states.btn_c = !gpio_get(13);
    states.states.btn_d = !gpio_get(14);
    states.states.aux_1 = !gpio_get(19);
    states.states.aux_2 = !gpio_get(20);
    states.states.aux_3 = !gpio_get(21);
    states.states.aux_4 = !gpio_get(12);
    states.states.aux_5 = !gpio_get(11);
    states.states.aux_6 = !gpio_get(10);
    report.buttons = states.raw;

    report.x = quad1_get_pos() * (65536.0f/80.0f);
    report.y = quad2_get_pos() * (65536.0f/80.0f);
    report.rx = 0;
    report.ry = 0;

    // Send the 3 bytes HID report
    if (tud_hid_ready())
        tud_hid_report(1, &report, sizeof(report));
}

void set_leds_task() {
    if(board_millis() % 5 == 0) return;
    ws2812.set_bfl(light_data.lights.strip[0], light_data.lights.strip[1], light_data.lights.strip[2]);
    ws2812.set_bfr(light_data.lights.strip[3], light_data.lights.strip[4], light_data.lights.strip[5]);
    ws2812.set_sl(light_data.lights.strip[6], light_data.lights.strip[7], light_data.lights.strip[8]);
    ws2812.set_sr(light_data.lights.strip[9], light_data.lights.strip[10], light_data.lights.strip[11]);
    ws2812.set_bl(light_data.lights.strip[12], light_data.lights.strip[13], light_data.lights.strip[14]);
    ws2812.set_br(light_data.lights.strip[15], light_data.lights.strip[16], light_data.lights.strip[17]);
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
