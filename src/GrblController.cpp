#include "GrblController.h"

GrblController::GrblController() 
    : m_serial(std::make_unique<SerialPortManager>()) {}

GrblController::~GrblController() {
    Disconnect();
}

bool GrblController::Connect(const std::string& portName, int baudRate) {
    if (m_serial->OpenPort(portName, baudRate)) {
        m_serial->StartAsyncRead([this](const std::string& line) {
            if (m_onMessageReceived) {
                m_onMessageReceived(line);
            }
        });
        return true;
    }
    return false;
}

void GrblController::Disconnect() {
    if (m_serial->IsOpen()) {
        m_serial->ClosePort();
    }
}

bool GrblController::IsConnected() const {
    return m_serial && m_serial->IsOpen();
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

std::vector<std::string> GrblController::GetAvailablePorts() {
    return m_serial->ScanPorts();
}