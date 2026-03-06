#ifndef GRBLCONTROLLER_H
#define GRBLCONTROLLER_H

#include "SerialPortManager.hpp"
#include <GrblTypes.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>
#include <map>
#include <optional>

enum ScanDirection {
    Horizontal, 
    Vertical   
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

    void MoveTo(std::optional<double> x, std::optional<double> y, std::optional<int> feedRate = std::nullopt);

    void JogTo(double x, double y, int feedRate);

    void SendCommand(const std::string &command);
    void SendRealtimeCommand(const std::string &command);
    void SetupMachineAndHome();

    void SetOnMessageReceived(MessageCallback callback);
    void SetOnStatusUpdate(StatusCallback callback);
    std::vector<std::string> GetAvailablePorts();

    std::map<std::string, std::string> GetSettings() const { return m_settings; }
    bool ParseSetting(const std::string& line);

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

    const double POS_TOLERANCE = 0.5;
};

#endif