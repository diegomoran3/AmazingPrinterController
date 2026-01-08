#include "SerialPortManager.h"

SerialPortManager::SerialPortManager()
    : serial_(ioContext_)
{}

SerialPortManager::~SerialPortManager() {
    ClosePort();
}

std::vector<std::string> SerialPortManager::ScanPorts() {
    std::vector<std::string> ports;
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
        std::string path = entry.path().string();
        if (path.find("ttyS") != std::string::npos ||
            path.find("ttyUSB") != std::string::npos ||
            path.find("ttyACM") != std::string::npos) {
            ports.push_back(path);
        }
    }
    return ports;
}

bool SerialPortManager::OpenPort(const std::string& portName, unsigned int baudRate) {
    try {
        if (serial_.is_open()) serial_.close();

        serial_.open(portName);

        serial_.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
        serial_.set_option(boost::asio::serial_port_base::character_size(8));
        serial_.set_option(boost::asio::serial_port_base::parity(
            boost::asio::serial_port_base::parity::none));
        serial_.set_option(boost::asio::serial_port_base::stop_bits(
            boost::asio::serial_port_base::stop_bits::one));
        serial_.set_option(boost::asio::serial_port_base::flow_control(
            boost::asio::serial_port_base::flow_control::none));

        int fd = serial_.native_handle();
        int flags;
        ioctl(fd, TIOCMGET, &flags);

        flags |= TIOCM_DTR;  // set DTR
        flags &= ~TIOCM_RTS; // clear RTS

        ioctl(fd, TIOCMSET, &flags);


        std::cout << "Opened serial port: " << portName << " at " << baudRate << " baud\n";

        Write("\r\n\r\n");

        // CRITICAL: Wait for Arduino to reset and flush any garbage
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error opening port " << portName << ": " << e.what() << "\n";
        return false;
    }
}

void SerialPortManager::ClosePort() {
    if (serial_.is_open()) serial_.close();
}

bool SerialPortManager::IsOpen() const {
    return serial_.is_open();
}

bool SerialPortManager::Write(const std::string& data) {
	return this->Write(boost::asio::buffer(data));
}

bool SerialPortManager::Write(const boost::asio::const_buffer& buffer) {
    if (!serial_.is_open()) return false;

    try {
        boost::asio::write(serial_, buffer);
        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Write error: " << e.what() << "\n";
        return false;
    }
}

void SerialPortManager::StartAsyncRead(std::function<void(const std::string&)> onLineRead) {
    m_onLineRead = onLineRead;
    DoRead();

    // Start the io_context in a separate thread so it doesn't block the GUI
    std::thread([this]() {
        auto workGuard = boost::asio::make_work_guard(ioContext_);
        ioContext_.run();
    }).detach();
}

void SerialPortManager::DoRead() {
    serial_.async_read_some(boost::asio::buffer(&m_readChar, 1),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            if (!ec) {
                if (m_readChar == '\n') {
                    if (m_onLineRead) m_onLineRead(m_inputBuffer);
                    m_inputBuffer.clear();
                } else if (m_readChar != '\r') {
                    m_inputBuffer += m_readChar;
                }
                DoRead(); // Wait for the next character
            }
        });
}
