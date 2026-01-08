#include "Application.h"

Application::Application() : _serial(std::make_shared<SerialPortManager>()), grblManager_(_serial)
{
    //ctor
}

int Application::run()
{
    int choice;

    while (true)
    {
        std::cout << "\n=== Main Menu ===\n";
        std::cout << "1. Connect to serial\n";
        std::cout << "2. Send Command\n";
        std::cout << "3. Exit\n";
        std::cout << "Choose an option (1-3): ";
        std::cin >> choice;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice)
        {
            case 1:
            {
                auto ports = _serial->ScanPorts();

                if (ports.empty())
                {
                    std::cout << "No serial connected\n" << std::endl;
                    break;
                }

                std::cout << "Available ports:\n";
                for (size_t i = 0; i < ports.size(); ++i) {
                    std::cout << i << ": " << ports[i] << "\n";
                }

                size_t choice;
                std::cout << "Select a port index: ";
                std::cin >> choice;

                if (choice >= ports.size()) {
                    std::cerr << "Invalid choice\n";
                    return 1;
                }

                if (!_serial->OpenPort(ports[choice])) break;

                break;
            }
            case 2:{
                if (!_serial->IsOpen()){
                    std::cout << "\nSerial is not open\n";
                    break;
                }

                _serial->Write("G0 X10 Y10\n");
                break;
            }
            case 3:{
                grblManager_.InitializeMachine();
                break;
            }
            case 4:{
                if (!_serial->IsOpen()){
                    std::cout << "\nSerial is not open\n";
                    break;
                }

                _serial->Write("$X\n");
                break;
            }
            case 5:{
                if (!_serial->IsOpen()){
                    std::cout << "\nSerial is not open\n";
                    break;
                }

                _serial->Write("\x18");
                _serial->ReadLine();

                break;
            }

            default:
                std::cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}

Application::~Application()
{
    //dtor
}
