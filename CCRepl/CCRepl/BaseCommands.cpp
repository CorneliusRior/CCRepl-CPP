#include "pch.h"
#include <ranges>
#include "CommandBuilder.h"
#include "CommandSet.h"
#include "fmt.h"
#include "ReplCommand.h"
#include "ReplContext.h"

namespace CCRepl {

	static void Handler(ReplContext& ctx, CommandArgs& args) {
		ctx.WriteLine("Not yet implemented.");
	}

	static void Help(ReplContext& ctx, CommandArgs& args) {

		// Create filtered map (shows all if string is empty):
		std::optional<std::string> inputKey = args.Get<std::string>(0);
		std::string ik = str::DotSeparated(inputKey.value_or(""));		// For display.
		std::string sk = str::ToLower(ik);								// Lower case, for searching.
		auto filtered = ctx.CommandReg | std::views::filter(
			[&sk](const auto& it) { return it.first.starts_with(sk); }
		);

		// Print announcement:
		std::size_t count = std::ranges::distance(filtered);
		if (count == 0) {
			ctx.WriteLine(std::format("No commands starting with '{}'. Help command only uses canonical command names, to find canonical name from an alias, try 'Help.Aliases(<alias>)'.", ik));
			return;
		}
		else {
			if (inputKey) ctx.WriteLine(std::format("Printing all commands starting with '{}' ({} total):\n", ik, count));
			else ctx.WriteLine(std::format("Printing all commands ({} total):\n", count));
		}

		// Some options contradict, e.g. full & oneline, so to avoid confusion, we just pay attention to first.
		if (args.Options.size() > 0) {
			std::string opt1 = str::TrimToLower(args.Options[0]);
			
			if (str::InVector(opt1, { "-oneline", "-ol" })) {
				for (const auto& it : filtered) ctx.WriteLine(str::TruncatePadRight(it.second.Address, 30) + str::Truncate(it.second.Desc.value_or("-"), 150));
				return;
			}

			if (str::InVector(opt1, { "-full", "-fl" })) {
				for (const auto& it : filtered) ctx.WriteLine(it.second.PrintFull() + '\n');
				return;
			}

			if (str::InVector(opt1, { "-usage", "-u" })) {
				for (const auto& it : filtered) ctx.WriteLine(str::TruncatePadRight(it.second.Address, 30) + str::Truncate(it.second.Usage.value_or("-"), 150));
				return;
			}

		}

		// No options: Print full if sk specified, print descriptions otherwise:
		if (inputKey) for (const auto& it : filtered) ctx.WriteLine(it.second.PrintFull());
		else for (const auto& it : filtered) ctx.WriteLine(str::TruncatePadRight(it.second.Address, 30) + str::Truncate(it.second.Desc.value_or("-"), 150));		

	}

	static void HelpAlias(ReplContext& ctx, CommandArgs& args) {

		// Create filtered map (shows all if string is empty):
		std::optional<std::string> inputKey = args.Get<std::string>(0);
		std::string ik = str::DotSeparated(inputKey.value_or(""));		// For display.
		std::string sk = str::ToLower(ik);								// Lower case, for searching.
		auto filtered = ctx.AliasReg | std::views::filter(
			[&sk](const auto& it) { return it.first.starts_with(sk); }
		);

		// Print announcement:
		std::size_t count = std::ranges::distance(filtered);
		if (count == 0) {
			ctx.WriteLine(std::format("No commands or aliases found starting with '{}'.", ik));
			return;
		}
		else {
			if (inputKey) ctx.WriteLine(std::format("Printing all command names and aliases starting with '{}' ({} total):\n", ik, count));
			else ctx.WriteLine(std::format("Print all command names and aliases ({} total):\n", count));
		}

		// Print:
		for (const auto& it : filtered) ctx.WriteLine(str::TruncatePadRight(it.first, 30) + str::Truncate(it.second->Address, 150));

		// End box:
		ctx.WriteLine();
		ctx.WriteLine(
			fmt::TxtBoxCenter(
				std::format("{} total aliases{}.",
					count,
					(inputKey.has_value() ? std::format(" starting with '{}' found", ik) : "")
				),
				inputKey.has_value() ? ik : "All"
			)
		);
	}

	static void HelpTree(ReplContext& ctx, CommandArgs& args) {
		std::optional<std::string> inputCmd = args.Get<std::string>(0);
		if (!inputCmd) ctx.WriteLine(ctx.RootTree());
		else ctx.WriteLine(ctx.FindCommand(str::DotSeparated(*inputCmd))->PrintTree("", ""));
	}

	static void Clear(ReplContext& ctx, CommandArgs& args) {
		if (args.Opt("-b")) ctx.Clear();
		else ctx.Clear(args.GetOr<std::string>(0, "Cleared Screen."));
	}

	static void Echo(ReplContext& ctx, CommandArgs& args) {
		std::string r = args.GetRequired<std::string>(0);
		for (std::size_t i = 0; i < args.GetRequired<std::size_t>(1); i++) {
			ctx.Write(r);
		}
		ctx.WriteLine();
	}

	static void Exit(ReplContext& ctx, CommandArgs& args) {
		ctx.CloseApp();
	}

	BaseCommands::BaseCommands() {
		Define(

			Cmd("Help")
			.Aliases("h", "?")
			.Exec(Help)
			.Args(StrArg("Search Key", ArgMode::Optional))
			.Options("-oneline", "-ol", "-full", "-fl", "-usage", "-u")
			.Desc("Lists all commands and descriptions, or shows full help for all commands with Search Key is specified.\nOptions '-full' and '-fl' will show full help statement regardless.\nOptions '-oneline' and '-ol' will list descriptions regardless.\nOptions '-usage' and '-u' will show usage statement instead of descriptions.")
			.Children(

				Cmd("Aliases")
				.Aliases( "a", "als", "alias" )
				.Exec(HelpAlias)
				.Args(StrArg("Search Key", ArgMode::Optional))
				.Desc("Lists all aliases and their canonical names for all commands, or for all commands and aliases starting with Search Key is specified."),

				Cmd("Tree")
				.Aliases( "t", "map", "rootmap" )
				.Exec(HelpTree)
				.Args(StrArg("Command Name", ArgMode::Optional))
				.Desc("Prints command tree, or command tree descended from a command if specified. Visualisation of command map/hierarchy.")

			),

			Cmd("Clear")
			.Aliases( "clr", "clearscreen" )
			.Exec(Clear)
			.Args(StrArg("Clear Message", ArgMode::Optional))
			.Options("-b")
			.Desc("Clears the screen, replacing it with optional message, or 'Cleared screen' by default.\nOption '-b' will make it blank, showing no message."),

			Cmd("Echo")
			.Aliases( "ech", "print", "write" )
			.Exec(Echo)
			.Args(
				StrArg("Message", ArgMode::RequiredPrompt),
				SztArg("n", ArgMode::Optional, 1)
			)
			.Desc("Echos specified text n times (default = 1)."),

			Cmd("Exit")
			.Aliases("ext", "quit", "close", "closeapp")
			.Exec(Exit)
			.Desc("Closes the program.")

		);
	}

}