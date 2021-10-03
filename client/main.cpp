#include <iostream>
#include <limits>
#include <boost/asio.hpp>
#include "project_exception.hpp"
#include "project_status.hpp"
#include "client.hpp"

/*
 * main function, runs the main loop for the client. calls the appropriate functions, and catches the exceptions.
 */
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
            std::cout << "30) Request for public key\n";
            std::cout << "40) Request for waiting messages\n";
            std::cout << "50) Send a text message\n";
            std::cout << "51) Send a request for symmetric key\n";
            std::cout << "52) Send your symmetric key\n";
            std::cout << "53) Send a file\n";
            std::cout << "0) Exit client\n";
            std::cout << "? ";
            if (!(std::cin >> input))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "invalid input. not an integer\n";
                continue;
            }

            try
            {
				switch (input)
				{
				case 10:
					client.register_request();
					break;
				case 20:
					client.client_list_request();
					break;
				case 30:
					client.get_public_key_request();
					break;
				case 40:
					client.pull_messages_request();
					break;
				case 50:
					client.send_text_message_request();
					break;
				case 51:
					client.send_symmetric_key_request();
					break;
				case 52:
					client.send_symmetric_key();
					break;
				case 53:
					client.send_file_request();
					break;
				case 0:
					return 0;
					break;
				default:
					std::cout << "invalid input. not a valid option\n";
					continue;
				}
            }
			catch (const RecoverableProjectException& e)
            {
				std::cout << "Caught RecoverableProjectException: (status=" << static_cast<std::underlying_type_t<ProjectStatus>>(e.status) << ").\n";
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

