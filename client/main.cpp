﻿#include <iostream>
#include <limits>
#include <boost/asio.hpp>
#include "project_exception.hpp"
#include "project_status.hpp"
#include "client.hpp"

int main(void)
{
    try
    {
        std::cout << "MessageU client at your service.\n";
        Client client;
        while (1)
        {
            int input;
            std::cout << "\n";
            std::cout << "10) Register\n";
            std::cout << "20) Request for clients list\n";
            std::cout << "0) Exit client\n";
            std::cout << "? ";
            if (!(std::cin >> input))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "invalid input. not an integer\n";
                continue;
            }

            switch (input)
            {
            case 10:
                client.register_request();
                break;
            case 20:
                client.client_list_request();
                break;
            case 0:
                return 0;
                break;
            default:
                std::cout << "invalid input. not a valid option\n";
                continue;
            }
        }
    }
    catch (const ProjectException& e)
    {
        std::cout << "Caught ProjectException: (status=" << static_cast<std::underlying_type_t<ProjectStatus>>(e.status) << ").\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "Caught std::exception: (e.what=" << e.what() << ").\n";
    }
    catch (...)
    {
        std::cout << "Caught unknown exception.\n";
    }
}

