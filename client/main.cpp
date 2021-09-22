#include <iostream>
#include <boost/asio.hpp>
#include "project_exception.hpp"
#include "project_status.hpp"

int main(void)
{
    try
    {

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

