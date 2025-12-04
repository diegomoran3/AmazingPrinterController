#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>

class SerialPortManager
{
    public:
        SerialPortManager();
        SerialPortManager& operator=(const SerialPortManager&) = default;
        virtual ~SerialPortManager();

        std::vector<std::string> ScanPorts();

        bool OpenPort(const std::string& portName, unsigned int BaudRate = 115200);

        void ClosePort();

        bool IsOpen() const;

        bool Write(const std::string& data);

        bool Write(const boost::asio::const_buffer &buffer);

        std::string ReadLine();

    protected:

    private:
        boost::asio::io_context ioContext_;
        boost::asio::serial_port serial_;
};

#endif // SERIALPORTMANAGER_H
