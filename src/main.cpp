#include <iostream>
#include "Core/Visitor.h"
#include "Core/Parser.h"

#include <sstream>

int main()
{
    auto visitor = Aleng::Visitor();

    while (true)
    {
        std::cout << "Aleng$ ";

        char buffer[2048];
        std::cin.getline(buffer, 2048);

        auto ss = std::stringstream(buffer);

        if (ss.str() == ".exit")
        {
            std::cout << "Exiting..." << std::endl;
            break;
        }

        try
        {
            auto parser = Aleng::Parser(ss.str());
            auto ast = parser.Parse();
            ast->Print(std::cout);
            std::cout << std::endl;

            std::cout << ast->Accept(visitor) << std::endl;
        }
        catch (std::runtime_error err)
        {
            std::cout << "Error: " << err.what() << std::endl;
        }
    }
}