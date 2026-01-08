#include "GrblController.h"
#include <regex>

wxBEGIN_EVENT_TABLE(GrblController, wxEvtHandler)
    EVT_TIMER(ID_POLL_TIMER, GrblController::OnTimer)
wxEND_EVENT_TABLE()

GrblController::GrblController() 
    : m_serial(std::make_unique<SerialPortManager>()) {
    m_pollTimer = std::make_unique<wxTimer>(this, ID_POLL_TIMER);
}

GrblController::~GrblController() {
    Disconnect();
}

bool GrblController::Connect(const std::string& portName, int baudRate) {
    if (m_serial->OpenPort(portName, baudRate)) {
        m_serial->StartAsyncRead([this](const std::string& line) {
            // Check if it's a status report starting with '<'
            if (!line.empty() && line[0] == '<') {
                ParseStatus(line);
                return;
            }
            if (m_onMessageReceived) m_onMessageReceived(line);
        });

        m_pollTimer->Start(50);
        return true;
    }
    return false;
}

void GrblController::Disconnect() {
    m_pollTimer->Stop();
    if (m_serial->IsOpen()) m_serial->ClosePort();
}

bool GrblController::IsConnected() const {
    return m_serial && m_serial->IsOpen();
}

void GrblController::OnTimer(wxTimerEvent& event) {
    if (IsConnected()) {
        // The '?' character is the real-time status query in GRBL
        m_serial->Write("?"); 
    }
}

void GrblController::ParseStatus(const std::string& line) {
    std::regex posRegex("MPos:([-+]?[0-9]*\\.?[0-9]+),([-+]?[0-9]*\\.?[0-9]+),([-+]?[0-9]*\\.?[0-9]+)");
    std::smatch match;

    if (std::regex_search(line, match, posRegex)) {
        GrblStatus status;
        status.x = std::stod(match[1]);
        status.y = std::stod(match[2]);
        status.z = std::stod(match[3]);
        
        // Find state (the part between < and |)
        size_t firstPipe = line.find('|');
        if (firstPipe != std::string::npos) {
            status.state = line.substr(1, firstPipe - 1);
        }

        if (m_onStatusUpdate) m_onStatusUpdate(status);
    }
}

void GrblController::MoveTo(double x, double y) {
    // GRBL move command: G0 (Rapid) or G1 (Linear)
    std::string gcode = "G0 X" + std::to_string(x) + " Y" + std::to_string(y);
    SendRawCommand(gcode);
}

void GrblController::SendRawCommand(const std::string& command) {
    if (IsConnected()) {
        // Ensure command ends with newline for GRBL
        std::string formatted = command;
        if (formatted.back() != '\n') formatted += "\n";
        m_serial->Write(formatted);
    }
}

void GrblController::SoftReset() {
    if (IsConnected()) {
        m_serial->Write("\x18"); // Ctrl+X is the GRBL soft reset char
    }
}

void GrblController::SetOnMessageReceived(MessageCallback callback) {
    m_onMessageReceived = callback;
}

void GrblController::SetOnStatusUpdate(StatusCallback callback) {
    m_onStatusUpdate = callback;
}

std::vector<std::string> GrblController::GetAvailablePorts() {
    return m_serial->ScanPorts();
}