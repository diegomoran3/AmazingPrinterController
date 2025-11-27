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
        virtual ~SerialPortManager();

        std::vector<std::string> ScanPorts();

        bool OpenPort(const std::string& portName, unsigned int BaudRate = 115200);

        bool InitializeMachine();

        void ClosePort();

        bool IsOpen() const;

        bool Write(const std::string& data);

        std::string ReadLine();

    protected:

    private:
        boost::asio::io_context ioContext_;
        boost::asio::serial_port serial_;
        boost::system::error_code readError_;
        std::string readBuffer_;

        bool readComplete_;
        void readComplete(const boost::system::error_code& error, size_t bytes_transferred);
};

#endif // SERIALPORTMANAGER_H
