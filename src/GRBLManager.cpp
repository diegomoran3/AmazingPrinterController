#include "GRBLManager.h"

GRBLManager::GRBLManager(std::shared_ptr<SerialPortManager> serial) : _serial(serial)
{
}

GRBLManager::~GRBLManager()
{
}

bool GRBLManager::InitializeMachine()
{
    if (!_serial->IsOpen())
    {
        std::cout << "Cannot initialize, port is not open\n";
        return false;
    }

    uint8_t resetByte = 0x18;  // Ctrl+X (soft reset)
	_serial->Write(boost::asio::buffer(&resetByte, 1));

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    std::string response = _serial->ReadLine();  // read first line
    std::cout << response << std::endl;

    while (true) {
        std::string line = _serial->ReadLine();
        if (line.empty()) break;  // GRBL usually ends with empty line
        std::cout << line << std::endl;
    }

    return true;
}