#include "common/tusb_common.h"
#include "device/usbd.h"

// Gamepad Report Descriptor Template
// with 16 buttons and 2 joysticks with following layout
// | Button Map (2 bytes) |  X | Y | Z | Rz
#define GAMECON_REPORT_DESC_GAMEPAD(...)                   \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                \
        HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),              \
        HID_COLLECTION(HID_COLLECTION_APPLICATION),        \
        __VA_ARGS__                                        \
            HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),         \
        HID_USAGE_MIN(1),                                  \
        HID_USAGE_MAX(12),                                 \
        HID_LOGICAL_MIN(0),                                \
        HID_LOGICAL_MAX(1),                                \
        HID_REPORT_COUNT(12),                              \
        HID_REPORT_SIZE(1),                                \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        /* now 4 constant bits for padding */                \
        HID_REPORT_COUNT(1),                              \
        HID_REPORT_SIZE(4),                                \
        HID_INPUT(HID_CONSTANT),                           \
        /* now the 2 joysticks */                              \
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),            \
        HID_LOGICAL_MIN(0x00),                             \
        HID_LOGICAL_MAX_N(0xffff, 2),                      \
        HID_USAGE(HID_USAGE_DESKTOP_X),                   \
        HID_USAGE(HID_USAGE_DESKTOP_Y),                   \
        HID_USAGE(HID_USAGE_DESKTOP_Z),                   \
        HID_USAGE(HID_USAGE_DESKTOP_VX),                   \
        HID_REPORT_COUNT(4),                               \
        HID_REPORT_SIZE(16),                                \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        HID_COLLECTION_END

#define GAMECON_REPORT_DESC_LIGHTS(...)                         \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                     \
        HID_USAGE(0x00),                                        \
        HID_COLLECTION(HID_COLLECTION_APPLICATION),             \
        __VA_ARGS__                                             \
            HID_REPORT_COUNT(18), /*12 button lights */ \
        HID_REPORT_SIZE(8),                                     \
        HID_LOGICAL_MIN(0x00),                                  \
        HID_LOGICAL_MAX_N(0x00ff, 2),                           \
        HID_USAGE_PAGE(HID_USAGE_PAGE_ORDINAL),                 \
        HID_USAGE_MIN(1),                                       \
        HID_USAGE_MAX(18),                                      \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),     \
        HID_USAGE_MIN(1),                                       \
        HID_USAGE_MAX(1),                                       \
        HID_INPUT(HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE),  \
        HID_COLLECTION_END
