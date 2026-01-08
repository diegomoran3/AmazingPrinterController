#include "GrblController.h"
#include <regex>
#include <chrono>

GrblController::GrblController() 
    : m_serial(std::make_unique<SerialPortManager>()) {}

GrblController::~GrblController() {
    Disconnect();
}

bool GrblController::Connect(const std::string& portName, int baudRate) {
    if (m_serial->OpenPort(portName, baudRate)) {

        SetupMachineAndHome();

        m_serial->StartAsyncRead([this](const std::string& line) {
            if (!line.empty() && line[0] == '<') {
                ParseStatus(line);
            }
            if (m_onMessageReceived) m_onMessageReceived(line);
        });

        // Start the background polling thread
        m_keepPolling = true;
        m_pollThread = std::thread(&GrblController::PollingThreadLoop, this);
        return true;
    }
    return false;
}

void GrblController::Disconnect() {
    m_keepPolling = false;
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    
    if (m_serial && m_serial->IsOpen()) {
        m_serial->ClosePort();
    }
}

bool GrblController::IsConnected() const {
    return m_serial && m_serial->IsOpen();
}

void GrblController::PollingThreadLoop() {
    while (m_keepPolling) {
        if (IsConnected()) {
            m_serial->Write("?");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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

void GrblController::SetupMachineAndHome() {
    SoftReset();

    // 1. Enable Homing and Soft Limits
    m_serial->Write("$22=1\n"); // Enable Homing Cycle
    m_serial->Write("$20=1\n"); // Enable Soft Limits

    // 2. Set Homing Direction to -X and -Y
    m_serial->Write("$23=3\n");

    // 3. Define the Max Travel (Soft Limit boundaries)
    m_serial->Write("$130=400\n"); // Max X travel 400mm
    m_serial->Write("$131=380\n"); // Max Y travel 380mm

    // 4. Unlock the machine 
    // Grbl starts in Alarm mode if homing is enabled
    m_serial->Write("$X\n");

    // 5. Start the Homing Cycle
    m_serial->Write("$H\n");
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