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

        m_serial->StartAsyncRead([this](const std::string& line) {
            if (!line.empty() && line[0] == '<') {
                ParseStatus(line);
                return;
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
            m_serial->Write(Grbl::StatusQuery);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

bool GrblController::ParseSetting(const std::string& line) {
    // Check if it is a setting line like $100=250.000
    if (line.empty() || line[0] != '$') return false;
    
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return false;

    // Extract ID ($100) and Value (250.000)
    std::string id = line.substr(0, eqPos);
    std::string val = line.substr(eqPos + 1);
    
    // Clean up comments if any (remove text after '(' )
    size_t commentPos = val.find('(');
    if (commentPos != std::string::npos) {
        val = val.substr(0, commentPos);
    }
    
    // Store in our permanent map
    m_settings[id] = val;
    return true;
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

void GrblController::SendCommand(const std::string& command) {
    if (IsConnected()) {
        std::string formatted = command;
        if (formatted.back() != '\n') {
            formatted += "\n";
        }
        m_serial->Write(formatted);
    }
}

void GrblController::SendRealtimeCommand(const std::string& command) {
    if (IsConnected()) {
        m_serial->Write(command);
    }
}

void GrblController::SendRawCommand(const std::string& command) {
    if (IsConnected()) {
        std::string formatted = command;
        if (formatted.back() != '\n') formatted += "\n";
        m_serial->Write(formatted);
    }
}

void GrblController::SetupMachineAndHome() {
    // 1. Enable Homing and Soft Limits
    m_serial->Write("$22=1\n"); // Enable Homing Cycle
    m_serial->Write("$20=1\n"); // Enable Soft Limits

    // 2. Set Homing Direction to -X and -Y
    m_serial->Write("$23=3\n");

    // 3. Define the Max Travel (Soft Limit boundaries)
    m_serial->Write("$130=400\n"); // Max X travel 400mm
    m_serial->Write("$131=380\n"); // Max Y travel 380mm
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