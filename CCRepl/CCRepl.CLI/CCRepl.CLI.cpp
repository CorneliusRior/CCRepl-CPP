
#include <iostream>
#include <Windows.h>
#include "ReplContext.h"
#include "CommandSet.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "CCRepl CLI (C++).\nType 'help' to see commands, type 'exit' to quit.\n";
    CCRepl::BaseCommands base;
    CCRepl::TestCommands test;

    CCRepl::ReplContext ctx( &base, &test );

    ctx.ReqReadLine = [](const std::string& prompt) {
        std::cout << prompt;
        std::string text;
        std::getline(std::cin, text);
        return text;
        };
    ctx.ReqWriteLine = [](const std::string& msg) { std::cout << msg << std::endl; };
    ctx.ReqWrite = [](const std::string& msg) { std::cout << msg; };
    ctx.ReqSetCaretVis = [](bool visible) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(h, &info);
        info.bVisible = visible;
        SetConsoleCursorInfo(h, &info);
        };
    ctx.ReqWriteStatus = [](std::string msg) { std::cout << '\r' << msg << std::flush; };
    ctx.ReqClearStatus = [](std::string msg) { std::cout << '\r' << msg << '\n' << std::flush; };

    while (ctx.running) {
        ctx.Write("> ");
        std::string line;
        std::getline(std::cin, line);
        ctx.Execute(line);
    }
    ctx.WriteLine("Closing");
    return 0;
}
