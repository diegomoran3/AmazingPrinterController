#ifndef MAIN_H
#define MAIN_H

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "SerialPortManager.h"

class main
{
    public:
        main();
        virtual ~main();

    protected:

    private:
        SerialPortManager serialPortManager;
};

#endif // MAIN_H
