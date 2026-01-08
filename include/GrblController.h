#ifndef GRBLCONTROLLER_H
#define GRBLCONTROLLER_H

#include "SerialPortManager.h"
#include <wx/timer.h>
#include <string>
#include <memory>
#include <functional>

// Structure to hold parsed data
struct GrblStatus {
    std::string state;
    double x, y, z;
};

class GrblController : public wxEvtHandler {
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
    void SoftReset();

    // Callbacks
    void SetOnMessageReceived(MessageCallback callback);
    void SetOnStatusUpdate(StatusCallback callback);

    std::vector<std::string> GetAvailablePorts();

private:
    void OnTimer(wxTimerEvent& event);
    void ParseStatus(const std::string& line);

    std::unique_ptr<SerialPortManager> m_serial;
    std::unique_ptr<wxTimer> m_pollTimer;
    
    MessageCallback m_onMessageReceived;
    StatusCallback m_onStatusUpdate;

    enum { ID_POLL_TIMER = 1000 };
    wxDECLARE_EVENT_TABLE();
};

#endif