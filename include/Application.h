#ifndef APPLICATION_H
#define APPLICATION_H

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "SerialPortManager.h"
#include "GRBLManager.h"
#include "memory"

class Application
{
    public:
        Application();
        virtual ~Application();
        int run();

    protected:

    private:
		GRBLManager grblManager_;
        std::shared_ptr<SerialPortManager> _serial;
};

#endif // APPLICATION_H
