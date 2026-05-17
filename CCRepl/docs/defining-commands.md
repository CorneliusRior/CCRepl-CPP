# Defining Commands
Commands are the building block for a CCRepl command system. 
Commands are defined as class `ReplCommand`, and built using `CommandBuilder`.
This document will describe the functionalities of `CommandBuilder` and provide examples and the end.

For information on how to set up a CCRepl environment, see [getting-started.md](getting-started.md).


## CommandBuilder Functions
While there is no technical reason to order these functions in this way, it is recommended that they follow the same order as presented here for clarity:

| Command | Syntax | Function |
| ------- | ------ | -------- |
| CommandBuilder | `CommandBuilder(std::string name)` | Create `CommandBuilder` instance and define canonical name. |
| Cmd | `Cmd(std::string name)` | Shortened function to create `CommandBuilder` instance and define canonical name. |
| Aliases | `.Aliases(Als&&... aliases)` | Add aliases to the function which can be called instead of canonical name. |
| Exec | `.Exec(std::function<void(ReplContext&, CommandArgs&)> exec)` | Define command handler (see [defining-command-handlers.md](defining-command-handlers.md)). |
| Test | `.Test(std::function<void(ReplContext&, CommandArgs&)> exec)` | Define test handler (see [defining-command-handlers.md](defining-command-handlers.md)). |
| Args | `.Args(CmdArg<T>&&... spec)` | Define argument structure. |
| Options | `.Options(Opt&&... options)` | Define a list of options to display in help commands: has no effect on function. |
| Mode | `.Mode(int mode)` | Define mode: an int which command handlers can see to allow multiple commands to use the same command handler in different ways. |
| Desc | `.Desc(std::string desc)` | Define description: Displayed in help commands. Should be brief, ~1-2 lines.
| LongDesc | `.LongDesc(std::string longDesc)` | Define long description: Displayed in help commands, for giving a more complete description of the commands functionality and other details. |
| Examples | `.Examples(Exp&&... examples)` | Define examples: Displayed in help commands, example inputs to demonstrate proper or potential usage.
| Group | `.Group(std::string group)` | Define group: if option `-g` is used in help commands, commands with identical group names are listed together. Case sensitive. |
| Children | `.Children(Cmds&&... cmds)` | Define child commands.

## Defining Names
A command only required one parameter: the name. 
Every other parameter is either optional or automatically generated.
For this reason, a skeleton for a larger command structure can be built using only names.

A commands **canonical name** is the official name of that command, as opposed to **aliases**.
The name does not include parent command names.
The full combination of command names is called the command **address**, and it automatically assigned.
Commands can be called by canonical address, or address composed of any combination of canonical and alias names.

For example, for the base command `Help.Tree()`:
- **Canonical name:** `Tree`.
- **Aliases:** `[ t, map, rootmap ]`.
- **Address:** `Help.Tree`.
- **Aliases Addresses:**
	- `h.tree`
	- `?.t`
	- `help.map`

`ReplCommand` can be built using the `CommandBuilder`, which can be called with the shortened function `Cmd()`.
Child `CommandBuilder` instances can be defined instide `CommandBuilder::Children()`.

### Name Example
Let's say we would like to add 3 commands under 'Data':
- Add
- Delete
- Edit

We can quickly define these like this:
```c++
DataCommands::DataCommands() {
	Define(		
		Cmd("Data")
		.Children(
			Cmd("Add"),
			Cmd("Delete"),	
			Cmd("Edit")
		)
	);
}
```

So long as the CommandSet `DataCommands` is being created (see [getting-started.md](getting-started.md)), these commands will show up on the help list, but they won't have any function yet:

```
> Help(Data) -o
Printing addresses and descriptions for all commands beginning with 'Data' (4 total):

Data
Data.Add
Data.Delete
Data.Edit
```

## Defining Arguments
### ArgSpec
Each command contains a `std::vector<std::unique_ptr<IArgSpec> ArgSpecs`, or "Argument Specification". An ArgSpec defines the arguments which a command expects. It includes:
- **Name** (used in exception handling)
- **Mode** (behaviour in the case of missing argument)
- **PmtInfo** (prompt info)
- **Fallback** (optional fallback value)

When a command is executed, these ArgSpecs are used to parse string input into a `CommandArg` object which is then passed to the command handler (see [defining-command-handlers.md](defining-command-handlers.md)).

### CmdArg
`CommandBuilder` does not accept `ArgSpec` directly, instead it accepts the struct `CmdArg<T>`.
A `CmdArg<T>` can be created directly, or through one of the building functions:
```c++
CmdArg<T>(std::string name, std::function<bool(const std::string&, T&)> parser, std::optional<T> fallback, PromptInfo pmtInfo);
```
- **IntArg():** Integer
- **DblArg():** Double
- **SztArg():** std::size_t
- **StrArg():** std::string
- **DtmArg():** std::tm

### Prompting
Every ArgSpec contains `PromptInfo`, which contains:
- **Prompt:** A string to prompt entry.
- **RetryPrompt:** A string to print if previous entry could not be parsed.
- **CancalStrings:** A list of strings which can be entered to revert to default or cancel the command.

By default, prompts are autogenerated, and the only cancel string is '\\'.

If an argument is missing, the `CommandArgs` object will behave in accordance with the `Mode` parameter, an enum called `ArgMode`:
- **`ArgMode::Required`:** Throws if not present.
- **`ArgMode::RequiredPrompt`:** Prompts for input until it can parse, unless input is in CancelStrings, in which case command is cancelled.
- **`ArgMode::Optional`:** Reverts to fallback (`std::nullopt` by default).
- **`ArgMode::OptionalPrompt`:** Prompts for input until it can parse, unless input is in CancelStrings, in which case it reverts to fallback (`std::nullopt` by default).

### Arguments Example
Let's give arguments to the three subcommands defined [earlier](###name-example):
```c++
DataCommands::DataCommands() {
	Define(		
		Cmd("Data")
		.Children(
			Cmd("Add")
			.Args(
				StrArg("Name", ArgMode::RequiredPrompt),
				IntArg("Age", ArgMode::OptionalPrompt, std::nullopt)
			)

			Cmd("Delete")
			.Args(IntArg("Id", ArgMode::RequiredPrompt)

			Cmd("Edit")
			.Args(IntArg("Id", ArgMode::RequiredPrompt)

		)
	);
}
```

## Full example
In practice most commands will not have this many parameters specified.
On addition is the subcommands `Name` and `Age` in `Data.Edit`, added to demonstrate `Mode()`.
For creation of command handlers, see [defining-command-handlers.md](defining-command-handlers.md).

```c++
DataCommands::DataCommands() {
	Define(		
		Cmd("Data")
		.Aliases("d", "dta")
		.Desc("Nodal command for 'Data' commands")
		.Group("Data")
		.Children(

			Cmd("Add")
			.Aliases("+", "a", "new", "addnew", "nw")
			.Exec(AddHandler)
			.Test(AddTest)			
			.Args(
				StrArg("Name", ArgMode::RequiredPrompt),
				IntArg("Age", ArgMode::OptionalPrompt, std::nullopt)
			)
			.Opts("-q")
			.Desc("Adds item to database.")
			.LongDesc("Adds an item to the database. Command behaviour can be altered by options:\n * '-q' ('quiet'): Adds without printing result.")
			.Examples("Data.Add(John, 25)", "d.+(James, 26) -q", "dta.new")
			.Group("Data")

			Cmd("Delete")
			.Aliases("-", "d", "del", "rm", "remove")
			.Exec(DeleteHandler)
			.Test(DeleteTest)
			.Args(IntArg("Id", ArgMode::RequiredPrompt)
			.Options("-f", "-q")
			.Desc("Deletes item from database.")
			.LongDesc("Deletes an item from the database. Uses (Y/N) confirmation beforehand. Command behaviour can be altered by options:\n * '-f' ('force'): Bypasses (Y/N) confirmation.\n * '-q' ('quiet'): Deletes withuot printing entry.")
			.Examples("Data.Delete(12)", "d.-(20) -f -q", "dta.rm")
			.Group("Data")

			Cmd("Edit")
			.Aliases("e", "edt", "update", "updt")
			.Exec(EditHandler)
			.Test(EditTest)
			.Args(IntArg("Id", ArgMode::RequiredPrompt)
			.Options("-f", "-q")
			.Mode(0)
			.Desc("Edits item in database.")
			.LongDesc("Edits an item in the database. Prompts for edit for each parameter, type '_' to keep unchanged. Uses (Y/N) confirmation before editing. Command behaviour can be altered by options:\n * '-f' ('force'): Bypasses (Y/N) confirmation.\n * '-q' ('quiet'): Edits withuot printing entry.")
			.Examples("Data.Edit(15)", "d.e(21) -f -q", "dta.updt")
			.Group("Data")
			.Children(

				Cmd("Name")
				.Aliases("n", "nm")
				.Exec(EditHandler)
				.Test(EditTest)
				.Args(
					IntArg("Id", ArgMode::RequiredPrompt),
					StrArg("New Name", ArgMode::RequiredPrompt)
				)
				.Options("-f", "-q")
				.Mode(1)
				.Desc("Edits item name.")
				.LongDesc("Edits an item name in the database. Uses (Y/N) confirmation before editing. Command behaviour can be altered by options:\n * '-f' ('force'): Bypasses (Y/N) confirmation.\n * '-q' ('quiet'): Edits withuot printing entry.")
				.Examples("Data.Edit.Name(30, Jacob)", "d.e.n(31, Josh) -f -q", "dta.updt.nm")
				.Group("Data"),

				Cmd("Age")
				.Aliases("a")
				.Exec(EditHandler)
				.Test(EditTest)
				.Args(
					IntArg("Id", ArgMode::RequiredPrompt),
					IntArg("New Age", ArgMode::RequiredPrompt)
				)
				.Options("-f", "-q")
				.Mode(2)
				.Desc("Edits item age.")
				.LongDesc("Edits an item afe in the database. Uses (Y/N) confirmation before editing. Command behaviour can be altered by options:\n * '-f' ('force'): Bypasses (Y/N) confirmation.\n * '-q' ('quiet'): Edits withuot printing entry.")
				.Examples("Data.Edit.Age(1, 20)", "d.e.a(27, 50) -f -q", "dta.updt.age")
				.Group("Data")
			)

		)
	);
}
```