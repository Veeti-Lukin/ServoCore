#include "control_api/windows/internal/ProgramUptimeClock.h"

namespace servo_core_control_api::windows::internal {

// Define the static member
std::chrono::time_point<std::chrono::high_resolution_clock> ProgramUptimeClock::start_time_point_ =
    std::chrono::high_resolution_clock::now();

uint64_t ProgramUptimeClock::uptimeMicroseconds() {
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start_time_point_).count();
}

uint64_t ProgramUptimeClock::uptimeMilliseconds() {
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_point_).count();
}

uint64_t ProgramUptimeClock::uptimeSeconds() {
    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now - start_time_point_).count();
}

}  // namespace servo_core_control_api::windows::internal