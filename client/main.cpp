#include <iostream>
#include <limits>
#include <boost/asio.hpp>
#include "project_exception.hpp"
#include "project_status.hpp"

int main(void)
{
    try
    {
        std::cout << "MessageU client at your service.\n\n";
        while (1)
        {
            int input;
            std::cout << "10) Register\n";
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
                std::cout << "in correct input\n";
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

