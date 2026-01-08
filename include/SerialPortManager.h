#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

// Add this line to silence the Boost bind warnings
#define BOOST_BIND_GLOBAL_PLACEHOLDER

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/optional.hpp>
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

        template <typename SyncReadStream, typename MutableBufferSequence>
        void ReadWithTimeout(SyncReadStream& s, const MutableBufferSequence& buffers, const boost::asio::deadline_timer::duration_type& expiry_time);
        void ClosePort();

        bool IsOpen() const;

        bool Write(const std::string& data);

        bool Write(const boost::asio::const_buffer &buffer);

        std::string ReadLine();

        void StartAsyncRead(std::function<void(const std::string&)> onLineRead);

    protected:

    private:
        boost::asio::io_context ioContext_;
        boost::asio::serial_port serial_;
        void DoRead();
        char m_readChar;
        std::string m_inputBuffer;
        std::function<void(const std::string&)> m_onLineRead;

};

#endif // SERIALPORTMANAGER_H
