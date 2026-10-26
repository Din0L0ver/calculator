#include "Parser.h"
#include "PluginLoader.h"
#include <iomanip>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
    try
    {
        OperationRegistry operations;
        PluginLoader().load("./plugins", operations, std::cerr);
        Parser parser(operations);
        Calculator calculator;
        std::cout << std::setprecision(15);
        if (argc > 1)
        {
            std::string expression;
            for (int i = 1; i < argc; ++i)
            {
                if (i > 1)
                {
                    expression += ' ';
                }
                expression += argv[i];
            }
            std::cout << calculator.execute(parser.parse(expression)) << '\n';
            return 0;
        }

        std::string line;
        while (std::getline(std::cin, line))
        {
            if (line == "exit" || line == "quit")
            {
                break;
            }
            if (line.find_first_not_of(" \t\r\n") == std::string::npos)
            {
                continue;
            }
            try
            {
                auto const program = parser.parse(line);
                std::cout << calculator.execute(program) << '\n';
            }
            catch (std::bad_alloc const&)
            {
                throw;
            }
            catch (std::exception const& exception)
            {
                std::cerr << "Error: " << exception.what() << '\n';
            }
            catch (...)
            {
                std::cerr << "Error: unknown exception\n";
            }
        }
        if (std::cin.bad())
        {
            throw std::runtime_error("Input stream failed");
        }
        return 0;
    }
    catch (std::exception const& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
    }
    catch (...)
    {
        std::cerr << "Error: unknown exception\n";
    }
    return 1;
}
