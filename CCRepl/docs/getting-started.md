# Getting Started

To create a REPL environment with CCRepl:
1. Generate `CommandSets`.
1. Create `ReplContext`.
1. Assign functionality to the following `ReplContext` events:
    - `ctx.ReqReadLine`
    - `ctx.ReqWriteLine`
    - `ctx.ReqReadLine`
    - `ctx.ReqSetVaretVis` (optional)
    - `ctx.ReqWriteStatus` (optional)
    - `ctx.ReqClearStatus` (optional)
1. Set up an input loop using `ctx.Execute()`, which runs as long as `ctx.running` is true.

It is recommended that you include `BaseCommands`, which contains `help()` and other commands.
A REPL environment with CCRepl can take any form so long as it contains `ReplContext` and functionality assigned to read and write events.

## Minimal Console App
Here is a minimal setup for a console application with a set of commands we call [`DataCommands`](##defining-command-sets):

```c++
#include <iostream>
#include <Windows.h>
#include "ReplContext.h"
#include "CommandSet.h"

int main()
{
    // UTF-8 output:
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "CCRepl CLI (C++).\nType 'help' to see commands, type 'exit' to quit.\n";
    
    // CommandSet(s):
    CCRepl::BaseCommands base;
    DataCommands data;

    // Create ReplContext with command sets as arguments:
    CCRepl::ReplContext ctx( &base, &data );

    // Assign functionality, ReqReadLine is used for prompting input.
    ctx.ReqReadLine = [](const std::string& prompt) {
        std::cout << prompt;
        std::string text;
        std::getline(std::cin, text);
        return text;
        };

    // ReqWriteLine (Write output ending with '\n'):
    ctx.ReqWriteLine = [](const std::string& msg) { std::cout << msg << std::endl; };

    // ReqWrite (Write output without '\n'):
    ctx.ReqWrite = [](const std::string& msg) { std::cout << msg; };

    // ReqSetCaretVis (Optional, set the caret invisible, used for status updating):
    ctx.ReqSetCaretVis = [](bool visible) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(h, &info);
        info.bVisible = visible;
        SetConsoleCursorInfo(h, &info);
        };

    // ReqWriteStatus (Rewrite the last line, used for things like wait spinners (Loading /)):
    ctx.ReqWriteStatus = [](std::string msg) { std::cout << '\r' << msg << std::flush; };

    // ReqClearStatus (Rewrite the last line, move onto the next line):
    ctx.ReqClearStatus = [](std::string msg) { std::cout << '\r' << msg << '\n' << std::flush; };

    // Main input loop:
    while (ctx.running) {
        ctx.Write("> ");
        std::string line;
        std::getline(std::cin, line);
        ctx.Execute(line);
    }
    ctx.WriteLine("Closing");
    return 0;
}
```

## Defining Command Sets
In the previous example, we use a CommandSet `DataCommands`, we will need to define this.
In a header file (e.g. `CommandSets.h`[^CommandSet.h], create a class of type  `CommandSet`:
```c++
class DataCommands : public CommandSet {
public:
    DataCommands();
};
```

Define `DataCommands()` like this:
```c++
DataCommands::DataCommands() {
    Define(

        // Commands go here...

    );
}
```

For information on how to define commands, see [defining-commands.md](defining-commands.md).

[^CommandSet.h]: Do not call this file `CommandSet.h`, as there is already a file in CCRepl with that name and this can cause conflicts (i.e. in `#include "CommandSet.h"`).