#ifndef COMMON_COLORS_H
#define COMMON_COLORS_H

#include "drivers/interfaces/RgbLedInterface.h"

namespace led_controller::common_colors {

using RGB = drivers::interfaces::RgbLedInterface::RGB;

constexpr RGB K_RED{255, 0, 0};
constexpr RGB K_PINK{255, 0, 127};
constexpr RGB K_PURPLE{255, 0, 255};
constexpr RGB K_BLUE{0, 0, 255};
constexpr RGB K_TURQUOISE{0, 0, 255};
constexpr RGB K_GREEN{0, 255, 0};
constexpr RGB K_YELLOW{255, 255, 0};
constexpr RGB K_ORANGE{255, 128, 0};
constexpr RGB K_WHITE{255, 255, 255};

constexpr RGB K_DISABLED{0, 0, 0};

}  // namespace led_controller::common_colors

#endif  // COMMON_COLORS_H
