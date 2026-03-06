#ifndef GRBL_TYPES_HPP
#define GRBL_TYPES_HPP

#include <string>

enum GrblState {
    Unknown,    // Default state before connection or first status report
    Idle,       // Machine is stationary and buffer is empty
    Run,        // Machine is moving or processing G-code
    Hold,       // Motion is paused (feed hold)
    Jog,        // Machine is in manual jogging mode
    Alarm,      // Machine is locked due to error (limit switch, etc.)
    Door,       // Safety door switch triggered
    Check,      // G-code check mode (simulating, not moving)
    Home,       // Homing cycle in progress ($H)
    Sleep       // Sleep mode
};

inline GrblState ParseStateString(const std::string& stateStr) {
    if (stateStr == "Idle") return GrblState::Idle;
    if (stateStr == "Run")  return GrblState::Run;
    if (stateStr == "Hold") return GrblState::Hold;
    if (stateStr == "Jog")  return GrblState::Jog;
    if (stateStr == "Alarm") return GrblState::Alarm;
    if (stateStr == "Door") return GrblState::Door;
    if (stateStr == "Check") return GrblState::Check;
    if (stateStr == "Home") return GrblState::Home;
    if (stateStr == "Sleep") return GrblState::Sleep;
    return GrblState::Unknown;
}

struct GrblStatus {
    GrblState state;
    double x, y, z;
};

namespace Grbl {
    const std::string SoftReset    = "\x18";
    const std::string StatusQuery  = "?";
    const std::string FeedHold     = "!";
    const std::string CycleStart   = "~";
    const std::string Unlock       = "$X";
    const std::string Home         = "$H";
}

#endif