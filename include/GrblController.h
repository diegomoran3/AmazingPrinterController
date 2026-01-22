#ifndef GRBLCONTROLLER_H
#define GRBLCONTROLLER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <map>
#include "SerialPortManager.h"

enum class GrblState {
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

enum Direction {
    DIR_Horizontal,
    DIR_Vertical
};

class GrblController {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using StatusCallback = std::function<void(const GrblStatus&)>;

    GrblController();
    ~GrblController();

    bool Connect(const std::string& portName, int baudRate = 115200);
    void Disconnect();
    bool IsConnected() const;

    void MoveTo(double x, double y);
    void MoveTo(double x, double y, double feedRate);

    void SendCommand(const std::string &command);
    void SendRealtimeCommand(const std::string &command);
    void SetupMachineAndHome();

    void SetOnMessageReceived(MessageCallback callback);
    void SetOnStatusUpdate(StatusCallback callback);
    std::vector<std::string> GetAvailablePorts();

    std::map<std::string, std::string> GetSettings() const { return m_settings; }
    bool ParseSetting(const std::string& line);

    void StartScanCycle(double startX, double startY, 
    int rows, int cols, 
    double stepX, double stepY, 
    std::function<void(int, int, double, double)> onPointReached,
    Direction direction,
    double speed = 6000);

    const double POS_TOLERANCE = 0.05;

    void WaitForArrival(double targetX, double targetY, double timeoutSecs = 10.0);

private:
    void PollingThreadLoop();
    void ParseStatus(const std::string& line);

    std::unique_ptr<SerialPortManager> m_serial;

    std::map<std::string, std::string> m_settings;
    
    // Threading members
    std::thread m_pollThread;
    std::atomic<bool> m_keepPolling{false};

    MessageCallback m_onMessageReceived;
    StatusCallback m_onStatusUpdate;

    GrblStatus m_currentStatus;
};

#endif