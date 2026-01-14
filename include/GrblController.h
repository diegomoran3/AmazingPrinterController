#ifndef GRBLCONTROLLER_H
#define GRBLCONTROLLER_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include "SerialPortManager.h"

struct GrblStatus {
    std::string state;
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

class GrblController {
public:
    using MessageCallback = std::function<void(const std::string&)>;
    using StatusCallback = std::function<void(const GrblStatus&)>;

    GrblController();
    ~GrblController();

    bool Connect(const std::string& portName, int baudRate = 115200);
    void Disconnect();
    bool IsConnected() const;

    void SendRawCommand(const std::string& command);
    void MoveTo(double x, double y);
    void SendCommand(const std::string &command);
    void SendRealtimeCommand(const std::string &command);
    void SetupMachineAndHome();

    void SetOnMessageReceived(MessageCallback callback);
    void SetOnStatusUpdate(StatusCallback callback);
    std::vector<std::string> GetAvailablePorts();

private:
    void PollingThreadLoop();
    void ParseStatus(const std::string& line);

    std::unique_ptr<SerialPortManager> m_serial;
    
    // Threading members
    std::thread m_pollThread;
    std::atomic<bool> m_keepPolling{false};

    MessageCallback m_onMessageReceived;
    StatusCallback m_onStatusUpdate;
};

#endif