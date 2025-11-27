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

        std::cout << "Opened serial port: " << portName << " at " << baudRate << " baud\n";

        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Error opening port " << portName << ": " << e.what() << "\n";
        return false;
    }
}

bool SerialPortManager::InitializeMachine()
{
    if (!serial_.is_open())
    {
        std::cout << "Cannot initialize, port is not open\n";
        return false;
    }

    unsigned char resetByte = 0x18;  // Ctrl+X (soft reset)
    boost::asio::write(serial_, boost::asio::buffer(&resetByte, 1));

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    std::string response = ReadLine();  // read first line
    std::cout << response << std::endl;

    while (true) {
        std::string line = ReadLine();
        if (line.empty()) break;  // GRBL usually ends with empty line
        std::cout << line << std::endl;
    }

    return true;
}

void SerialPortManager::ClosePort() {
    if (serial_.is_open()) serial_.close();
}

bool SerialPortManager::IsOpen() const {
    return serial_.is_open();
}

bool SerialPortManager::Write(const std::string& data) {
    if (!serial_.is_open()) return false;

    try {
        boost::asio::write(serial_, boost::asio::buffer(data));
        return true;
    } catch (const boost::system::system_error& e) {
        std::cerr << "Write error: " << e.what() << "\n";
        return false;
    }
}

std::string SerialPortManager::ReadLine() {
    readBuffer_.clear();
    readComplete_ = false;
    readError_ = boost::system::error_code();

    boost::asio::async_read_until(serial_,
    boost::asio::dynamic_buffer(readBuffer_), '\n',
    boost::bind(&SerialPortManager::readComplete, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));

    return readBuffer_;
}

void SerialPortManager::readComplete(const boost::system::error_code& error, size_t bytes_transferred)
{
    readComplete_ = true;
    readError_ = error;
}
