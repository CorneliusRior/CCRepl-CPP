# Defining Command Handlers
Command handlers are functions which commands call when executed. 
There are two types of command handler: 
- **Execute:** `std::funcion<void(ReplContext&, CommandArgs&)>`
- **Test:** `std::function<bool(ReplContext&, CommandArgs&)>`

For information on how to define commands, see [defining-commands.md](defining-commands.md). 
For information on how to set up a CCRepl environment, see [getting-started.md](getting-started.md).

## Setting up a command handler
A command handler function needs to accept arguments `ReplContext&` and `CommandArgs&`.
Here is the usual structure:

```c++
static void CmdHandler(ReplContext& ctx, CommandArgs& args) {	
	// ...
}
```

`ReplCommand.h` includes macros `CMD_H` and `CMD_T` ("Handler" & "Test") to make this a bit easier:
```c++
CMD_H(CmdHandler) {
	// ...
}
```

# ReplContext features
## Input & output
- **ctx.Write():** Writes string in output (invokes `ReqWrite`).
	- `void ctx.Write(const std::string& text)`
- **ctx.WriteLine():** Writes a line in output followed by `\n` (invokes `ReqWriteLine`).
	- `void ctx.WriteLine(const std::string& text)`
- **ctx.ReadLine():** Prints prompt and awaits input (invokes `ReqReadLine`).
	- `std::string ctx.ReadLine(const std::string& prompt = "")`
- **ctx.Confirm():** Requests (Y/N) confirmation.
	- `bool ctx.Confirm(const std::string& prompt = "(Y/N): ", const std::string& retryPrompt = "Cannot parse, please try again")`
- **ctx.WriteStatus():** Sets status (last line, which can be overridden, used for things like wait spinner).
	- `void ctx.WriteStatus(std::string text)`
- **ctx.ClearStatus():** Clears status and moves on to the next line.
	- `void ctx.ClearStatus(std::string text)`;

```c++
CMD_H(CmdHandler) {
	ctx.Write("Hello ");
	ctx.WriteLine("World!");
	ctx.WriteLine();
	std::string name = ctx.ReadLine("Enter name: ");
	if (ctx.Confirm(std::format("Use this name? '{}' (Y/N): ", name))) {
		ctx.SetCaretVis(false);
		for (std::size_t i = 0; i < 10; i++) {
			ctx.WriteStatus("Hello");
			std::this_thread::sleep_for(std::chrono::seconds(1));
			ctx.WriteStatus(name);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		ctx.ClearStatus("Hello World!");
		ctx.SetCaretVis(true);
	}
}
```

## Other
- **ctx.GetService():** Get access to some external service.
	- `std::shared_ptr<T> ctx.GetService()`
- **ctx.SetCaretVis():** Set the caret (or cursor) as visible or invisible (invokes `ReqSetCaretVis`. Do this before setting status).
	- `void ctx.SetCaretVis(bool visible)`
- **ctx.WaitSpinner():** Sets status with message and spinner (`Loading /`) while waiting for some task.
	- `T ctx.WaitSpinner(std::future<T> ft, const std::string& message = "Processing", const std::string& doneMessage = "Done.")`
	- `T ctx.WaitSpinner(std::function<T()> func, ..)`
	- Or use macro `CTX_WAIT_SPIN(T, function, message, doneMessage)`


```c++
DataService data;
ctx.AddService<DataService>(std::make_shared<DataService>(data));
```

```c++
CMD_H(CmdHandler) {
	auto data = ctx.GetService<DataService>();

	ctx.SetCaretVis(false);
	int r = CTX_WAIT_SPIN(int, data->CalculateInt(), "Processing", "Done:");
	ctx.SetCaretVis(true);
}
```

# CommandArgs features

## Getting Arguments
Arguments come in the form of `std::vector<std::unique_ptr<IArgValue>> Args`, which is public. There are three built-in options for getting arguments:
- **Get():** For optional arguments, returns `std::optional<T>`.
    - `std::optional<T> Get(std::size_t pos)`
- **GetOr():** For optional arguments, returns T or some fallback.
	- `T GetOr(std::size_t pos, T fallback)`
- **GetRequired():** For required arguments, or optional arguments where fallback is set, or generally confident that the value is present.
	- `T GetRequired(std::size_t pos)`

### Example
```c++
CMD_H(CmdHandler) {
	std::optional<int> a = args.Get<int>(0);
	std::string b = args.GetOr<std::string>(1, "Not present");
	double c = args.GetRequired<double>(2);
}
```

## Getting Options
Options come in the form of `std::vector<std::string> Options`, which is public. There are six built-in options for getting options (all are case insensitive):
- **HasOption():** Returns true if the given string is present in the options vector.
	- `bool HasOption(const std::string& opt)`
- **HasOptStart():** Returns true if any string in the options vector starts with the given string.
	- `bool HasOptionStart(const std::string& opt)`
	- Useful for shortened options (e.g. `-f` or `-force` both work).
- **Opt():** Returns true if any given string is present in the options vector.
	- `bool Opt(const Ts&... opts)`
	- Useful for aliases.
- **OptStrt():** Returns true if any string in the options vector starts with one of the given strings.
	- `bool OptStrt(const Ts&... opts)`
- **FirstOptionOf():** Given a list of strings, returns the position of the first of those string to appear in the options vector. If none appear, returns -1. 
	- `int FirstOptionOf(const Ts&... opts)`
	- Useful for switches for multiple functions, and cases where several options are mutually exclusive.
- **FirstOptionStart():** Given a list of strings, return the position of the first of those strings to have an option in the options vector start with that string. If none appear, returns -1.
	- `int FirstOptionStart(const Ts&... opts)`

```c++
CMD_H(CmdHandler) {
	bool a = args.HasOption("-a");			// Accepts "-a", not "-aa"
	bool b = args.HasOptionStart("-b");		// Accepts "-b" and "-bb"
	bool abc = args.Opt("-a", "-b", "-c");		

	int first = args.FirstOptionOf("-a", "-b", "-c", "-d", "-e", "-f");
	switch (first) {
		case 0: ctx.WriteLine("A"); break;
		case 1: ctx.WriteLine("B"); break;
		// ...
	}
}
```

## Getting Mode
`Mode` is an integer each command has, which can allow multiple commands to use the same handler with slight variations in function.
By default, it is 0.

The mode of the current command can be found with `int CommandArgs::Mode()`.

Alternatively, `bool CommandArgs::IsMode(int mode)`

```c++
CMD_H(CmdHandler) {
	int mode = args.Mode();
	switch (mode) {
		case 0:
			// ...
		case 1:
			// ...
		case 2:
			// ...
		default:
			// ...
	}

	if (args.IsMode(-1)) {
		// ...
	}
}
```

# Example
Continuing from the "Data" command example from [defining-commands.md](defining-commands.md), here is what the `EditHandler` could look like:

```c++
CMD_H(EditHandler) {
	auto service = ctx.GetService<DataService>();
	DataEntry entry = service->GetById(args.GetRequired<int>(0));
	DataEntry entryOld = entry;

	switch (args.Mode()) {

		case 1:
			entry.Name = args.GetRequired<std::string>(1);			
			break;
		case 2:
			entry.Age = args.GetRequired<int>(1);
			break;
		default:
			std::string newName = ctx.ReadLine("Enter new name, or '_' to keep old name");
			if (newName != "_") entry.Name = newName;

			int newAge;
			while (true) {
				std::string newAgeStr = ctx.ReadLine("Enter new age, or '_' to keep old age");
				if (newAgeStr == "_") break;
				if (parsers::TryInt(newAgeStr, newAge)) {
					entry.Age = newAge;
					break;
				}
			}
			break;

	}

	if (args.OptStrt("-f", "-q") || ctx.Confirm(std::format("Edit entry from \n{}\nto\n{}\nAre you sure? (Y/N): ", entryOld.Print(), entry.Print())))
		service->Update(entry);
	if (!args.HasOptionStart("-q")) ctx.WriteLine(std::format("Updated entry:\n{}", entry.Print()));
}
```