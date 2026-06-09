
#include <iostream>
#include <Windows.h>
#include <CCRepl/ReplContext.h>
#include <CCRepl/BaseCommands.h>
#include <CCRepl/ScptCommands.h>

std::string about =  R"(CCRepl CLI (C++)
2026, Cornelius Riordan.

Type 'help' to see commands, type 'exit' to quit.

This is a basic command-line interface for the CCRepl library, used for testing basic functions.
CCRepl is a library for building REPL command environments. It is designed to make the construction of command systems as quick and easy as possible. It features a hierarchical command structure, automatic command registration, command argument parsing and validation, and built-in base commands including help functions, command testing functions, and scripting capabilities for the execution of multiple commands in sequence.

For documentation and other information, see the CCRepl (C++) GitHub page: https://github.com/CorneliusRior/CCRepl-CPP.
)";

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "CCRepl CLI (C++).\nType 'help' to see commands, 'exit' to quit, 'about' for more info.\n";
    CCRepl::BaseCommands base;
    CCRepl::ScptCommands scpt;
    CCRepl::TestCommands test;

    CCRepl::ReplContext ctx( &base, &scpt, &test );
    
    // Services (Script):
    CTX_ADD_SVC(CCRepl::ScriptService, &ctx, std::filesystem::current_path() / ".." / ".." / ".." / "scripts");
    
    ctx.AboutStr = about;
    ctx.MaxWidth = 160;
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
