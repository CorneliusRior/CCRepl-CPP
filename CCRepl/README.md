# CCRepl (C++)
CCRepl is a C++ static library for building REPL-style command systems. Commands are registered in a hierarchical tree, and can be executed via external string input, directly within the program, or by running scripts.

Default command syntax follows the structure: `Command.Subcommand(arg1, arg2, arg3) -option1 -option2`.

CCRepl is designed to help quickly and easily create interactive command environments.
Includes tools to help parse and validate arguments and format output strings, including truncation, indexes, textboxes and tables.

There is also a version of CCRepl written in C#: [https://github.com/CorneliusRior/CCRepl-CS](https://github.com/CorneliusRior/CCRepl-CS).

More documentation:
- [Getting Started](docs/getting-started.md)
- [Defining Commands](docs/defining-commands.md)
- [Defining Command Handlers](docs/defining-command-handlers.md)

## Features
- Hierarchical command structure.
- Automatic command registration.
- Command Aliases.
- Built-in help functions.
- Automatic argument parsing.
- Prompt for missing arguments.
- Command options for flexible command behaviour.
- Optional test handler for commands.
- Embeddable I/O callbacks.
- Execute multiple commands in sequence with scripts.

## Example

```C++
CMD_H(Greet) {
	std::string name = args.GetRequired<std::string>(0);
	if (args.Opt("-loud")) ctx.WriteLine("HELLO " + name + "!");
	else ctx.WriteLine("Hello " + name + ".");
}

GreetCommands::GreetCommands() {
	Define(
		
		Cmd("Greet")
		.Aliases("g", "hello")
		.Exec(Greet)
		.Args(StrArg("Name", CCRepl::ArgMode::RequiredPrompt))
		.Options("-loud")
		.Desc("Greets a named person.")
		.Examples("Greet(Ada)", "hello(Ada) -loud")

	);
}
```
