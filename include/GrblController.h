#ifndef GRBLCONTROLLER_H
#define GRBLCONTROLLER_H

#include "SerialPortManager.h"
#include <string>
#include <memory>
#include <functional>

class GrblController {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    GrblController();
    ~GrblController();

    bool Connect(const std::string& portName, int baudRate = 115200);
    void Disconnect();
    bool IsConnected() const;

    // High-level GRBL commands
    void MoveTo(double x, double y);
    void SendRawCommand(const std::string& command);
    void SoftReset();

    // Set a callback to handle incoming data from GRBL
    void SetOnMessageReceived(MessageCallback callback);

    std::vector<std::string> GetAvailablePorts();

private:
    std::unique_ptr<SerialPortManager> m_serial;
    MessageCallback m_onMessageReceived;
};

#endif