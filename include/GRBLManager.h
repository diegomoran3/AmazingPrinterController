#ifndef GRBLMANAGER_H
#define GRBLMANAGER_H

#include "SerialPortManager.h"
#include <boost/asio.hpp>
#include <string>

class GRBLManager
{
public:
	GRBLManager(std::shared_ptr<SerialPortManager> serial);
	~GRBLManager();

	bool InitializeMachine();

protected:

private:
	std::shared_ptr<SerialPortManager> _serial;
};



#endif // GRBLMANAGER_H