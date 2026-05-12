
#include <iostream>
#include "ReplContext.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "CCRepl.CLI. Type 'help' to see commands.\n";
    CCRepl::BaseCommands base;

    CCRepl::ReplContext ctx( &base );

    ctx.ReqReadLine = [](const std::string& prompt) {
        std::cout << prompt;
        std::string text;
        std::getline(std::cin, text);
        return text;
        };
    ctx.ReqWriteLine = [](const std::string& msg) { std::cout << msg << std::endl; };
    ctx.ReqWrite = [](const std::string& msg) { std::cout << msg; };

    while (ctx.running) {
        ctx.Write("> ");
        std::string line;
        std::getline(std::cin, line);
        ctx.Test(line, true);
    }
    ctx.WriteLine("Closing");
    return 0;
}
