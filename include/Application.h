#ifndef APPLICATION_H
#define APPLICATION_H

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "SerialPortManager.h"

class Application
{
    public:
        Application();
        virtual ~Application();
        int run();

    protected:

    private:
        SerialPortManager serial_;
};

#endif // APPLICATION_H
