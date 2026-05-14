#include "pch.h"
#include <ranges>
#include <set>
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

		std::size_t count = std::ranges::distance(filtered);
		if (count == 0) {
			ctx.WriteLine(std::format("No commands found starting with '{}'. Try 'Help.Alias' for possible aliases.", ik));
			return;
		}

		// Determine mode:
		std::optional<bool> oneline;
		bool olDefault;
		int olOpt = args.FirstOptionStart("-o", "-ol", "-m");
		switch (olOpt) {
		case 0:
		case 1: oneline = true;				break;
		case 2: oneline = false;			break;
		default: oneline = std::nullopt;	break;
		}

		HelpAttribute att;
		int attMode = args.FirstOptionStart("-a", "-d", "-e", "-f", "-l", "-u");
		switch (attMode) {
		case 0:		att = HelpAttribute::Aliases;			 olDefault = true;	break;
		case 1:		att = HelpAttribute::Description;		 olDefault = true;	break;
		case 2:		att = HelpAttribute::Examples;			 olDefault = false;	break;
		case 3:		att = HelpAttribute::Full;				 olDefault = false;	break;
		case 4:		att = HelpAttribute::LongDescription;	 olDefault = false;	break;
		case 5:		att = HelpAttribute::Usage;				 olDefault = true;	break;
		default:	
			att = inputKey.has_value() ? HelpAttribute::Full : HelpAttribute::Description;
			olDefault = true; 
			break;
		}

		bool group = args.HasOptStart("-g");

		// Define col here w/ max, right now just '30';
		std::size_t col = 0;
		for (const auto& it : filtered) if (it.first.size() > col) col = it.first.size();
		col += 5;
		std::size_t total = 180 - col;
		std::ostringstream oss;

		// Write it out:
		if (group) {
			std::set<std::optional<std::string>> seen;
			std::vector<std::optional<std::string>> groups;
			for (const auto& it : filtered) {
				std::optional<std::string> group = it.second.Group;
				if (seen.insert(group).second) groups.push_back(group);
			}

			// Sort groups:
			std::sort(groups.begin(), groups.end(),
				[](const auto& a, const auto& b) {
					auto rank = [](const auto& s) {
						if (s && *s == "Base") return 0;
						if (s) return 1;
						return 2;
						};
					int ra = rank(a);
					int rb = rank(b);
					if (ra != rb) return ra < rb;
					return a < b;
				});

			for (std::optional<std::string> g : groups) {
				auto gfiltered = ctx.CommandReg | std::views::filter(
					[&sk, &g](const auto& it) { return it.first.starts_with(sk) && it.second.Group == g;}
				);

				// Print banner:
				oss << "\n * "
					<< g.value_or("Ungrouped")
					<< " ("
					<< std::ranges::distance(gfiltered)
					<< " total):\n";

				// Print each line:
				for (const auto& it : gfiltered)
					oss << it.second.PrintIndexLine(att, col, total, oneline.value_or(olDefault)) << '\n';
			}
		}
		else {
			for (const auto& it : filtered) 
				oss << it.second.PrintIndexLine(att, col, total, oneline.value_or(olDefault)) << '\n';
		}
		

		// Print announcement:
		std::string beginStmt = inputKey.has_value() ? std::format("beginning with '{}'", ik) : "";
		if (att == HelpAttribute::Full) ctx.WriteLine(std::format("Printing full information for all commands{} ({} total):\n", beginStmt, count));
		else ctx.WriteLine(std::format("Printing addresses and {} for all commands{} ({} total):\n", ToString(att), beginStmt, count));

		ctx.WriteLine(oss.str());
		// Test when you get back.
	}

	// Silly test, frankly you can disregard this one:
	static bool HelpTest(ReplContext& ctx, CommandArgs& args) {
		std::optional<std::string> inputKey = args.Get<std::string>(0);
		std::string ik = str::DotSeparated(inputKey.value_or(""));		// For display.
		std::string sk = str::ToLower(ik);								// Lower case, for searching
		auto filtered = ctx.CommandReg | std::views::filter(
			[&sk](const auto& it) { return it.first.starts_with(sk); }
		);
		std::size_t count = std::ranges::distance(filtered);
		if (count == 0) {
			ctx.WriteLine(std::format("No commands found starting with '{}'. Try 'Help.Alias' for possible aliases.", ik));
			return false;
		}
		else return true;
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
		
		// Define col.
		std::size_t col = 0;
		for (const auto& it : filtered) if (it.first.size() > col) col = it.first.size();
		col += 5;
		if (col > 90) col = 90;
		std::size_t col2 = 180 - col;

		std::ostringstream oss;

		if (args.HasOptStart("-g")) {
			std::set<std::optional<std::string>> seen;
			std::vector<std::optional<std::string>> groups;
			for (const auto& it : filtered) {
				std::optional<std::string> group = it.second->Group;
				if (seen.insert(group).second) groups.push_back(group);
			}

			// Sort groups:
			std::sort(groups.begin(), groups.end(),
				[](const auto& a, const auto& b) {
					auto rank = [](const auto& s) {
						if (s && *s == "Base") return 0;
						if (s) return 1;
						return 2;
						};
					int ra = rank(a);
					int rb = rank(b);
					if (ra != rb) return ra < rb;
					return a < b;
				});

			// Add each to string:
			for (std::optional<std::string> g : groups) {
				// Filter for group:
				auto gfiltered = ctx.AliasReg | std::views::filter(
					[&sk, &g](const auto& it) { return it.first.starts_with(sk) && it.second->Group == g;}
				);

				// Count:
				std::size_t gcount = 0;
				std::set<std::string> seenCmd;
				for (const auto& it : gfiltered) {
					std::string add = it.second->Address;
					if (seenCmd.insert(add).second) gcount++;
				}

				// Print banner:
				oss << "\n * " 
					<< g.value_or("Ungrouped") 
					<< " (" 
					<< std::ranges::distance(gfiltered) 
					<< " total aliases for " 
					<< gcount
					<< " commands) :\n";

				// Print each line:
				for (const auto& it : gfiltered) 
					oss << str::TruncatePadRight(it.first, col) + str::Truncate(it.second->Address, col2) << '\n';
			}
		}
		else for (const auto& it : filtered)
				oss << str::TruncatePadRight(it.first, col) + str::Truncate(it.second->Address, col2) << '\n';
		
		// Print banner:
		if (inputKey) ctx.WriteLine(std::format("Printing all command names and aliases starting with '{}' ({} total):\n", ik, count));
		else ctx.WriteLine(std::format("Print all command names and aliases ({} total):\n", count));
		
		// Print:
		ctx.WriteLine(oss.str());

		// Count unique commands:
		std::size_t uniqueCmd = 0;
		std::set<std::string> seenCmd;
		for (const auto& it : filtered) {
			std::string add = it.second->Address;
			if (seenCmd.insert(add).second) uniqueCmd++;
		}

		// End box:
		ctx.WriteLine();
		std::ostringstream report;
		report << count << " total aliases ";
		if (inputKey.has_value()) report << "starting with '" << ik << "' found ";
		report << "for " << uniqueCmd << " commands.";
		ctx.WriteLine(fmt::TxtBoxCenter(report.str(), inputKey.has_value() ? ik : "All"));
	}

	static void HelpTree(ReplContext& ctx, CommandArgs& args) {
		std::optional<std::string> inputCmd = args.Get<std::string>(0);
		if (!inputCmd) ctx.WriteLine(ctx.RootTree());
		else ctx.WriteLine(ctx.FindCommand(str::DotSeparated(*inputCmd))->PrintTree("", ""));
	}

	static void CommandList(ReplContext& ctx, CommandArgs& args) {
		std::optional<std::string> inputKey = args.Get<std::string>(0);
		std::string ik = str::DotSeparated(inputKey.value_or(""));		// For display.
		std::string sk = str::ToLower(ik);								// Lower case, for searching.
		auto filtered = ctx.CommandReg | std::views::filter(
			[&sk](const auto& it) { return it.first.starts_with(sk); }
		);
		std::size_t count = std::ranges::distance(filtered);
		if (count == 0) {
			ctx.WriteLine(std::format("No commands found starting with '{}'. Try 'Help.Alias' for possible aliases.", ik));
			return;
		}
		if (inputKey) ctx.WriteLine(std::format("Printing all commands beginning with '{}' ({} total):\n", ik, count));
		else ctx.WriteLine(std::format("Printing all commands ({} total):\n", count));

		std::ostringstream oss;
		for (const auto& it : filtered) oss << it.first << '\n';
		ctx.WriteLine(oss.str());
	}

	static void TestInput(ReplContext& ctx, CommandArgs& args) {
		std::string input = args.GetRequired<std::string>(0);
		ctx.Test(input, args.HasOptStart("-r"));
	}

	static void Script(ReplContext& ctx, CommandArgs& args) {
		// just occured to me that "mode" should be in commandargs instead of ctx, will change that.
		ctx.WriteLine("*Scripting is WIP.*");
		ctx.WriteLine("For the time being, we will just import the text and then print it:");
		std::string scriptTxt = str::ReadTextFile(args.GetRequired<std::string>(0));
		ctx.WriteLine(scriptTxt);

		ctx.WriteLine("\n\n***End of Script***\n\nNow we will try to parse it:\n");
		std::vector<ScriptStatement> sstmtList = TokenizeScript(scriptTxt);
		ctx.WriteLine(PrintSStmtList(sstmtList));

		ctx.WriteLine("\n\n***Parse MetaData:***\n");
		ScriptMetaData metadata = (sstmtList[0].tokens.args);
		ctx.WriteLine(metadata.Print());
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
			.Options("-a", "-d", "-e", "-f", "-g", "-l", "-m", "-o", "-u")
			.Desc("Lists all commands and descriptions, or shows full help for all commands with Search Key is specified.")
			.LongDesc(
				R"(Lists all commands and descriptions, or full help for all commands starting with Search Key. Behaviour altered with options:
 * '-a' ('aliases'): Prints list of all aliases for that command node (to see full list of all possible combinations, see Help.Alias).
 * '-d' ('description'): Prints full description without truncation.
 * '-e' ('example'): Prints example usages.
 * '-f' ('full'): Prints full info regardless of search key presence.
 * '-g' ('group'): Prints by group (by default only done with no search term. Use '-o' to ungroup that)
 * '-l' ('longdescription'): Prints full long description without truncation.
 * '-m' ('multiline'): Prints in multiple liens regardless of parameter (description by default, use '-f' to show full info).
 * '-o' ('oneline', also '-ol'): Prints only one line regardless of search key presence of parameter.
 * '-u' ('usage'): Prints usage statements instead of description.
Checks for options with 'startswith'. Only the first valid options is used (except for '-g').)"
			)
			.Examples( "Help", "?", "Help(Diary.Add)", "Help(Diary) -usage", "Help() -d -m" )
			.Group("Base")
			.Children(

				Cmd("Aliases")
				.Aliases( "a", "als", "alias" )
				.Exec(HelpAlias)
				.Args(StrArg("Search Key", ArgMode::Optional))
				.Options("-g")
				.Desc("Lists all aliases and corresponding canonical names for all commands, or for all commands and aliases starting with Search Key is specified.")
				.LongDesc("Lists all aliases and their canonical names for all commands, or for all commands and aliases starting with Search Key is specified.Behaviour altered with option : \n * '-g' ('group') : Prints by group.")
				.Examples( "Help.Aliases", "Help.als", "Help.Aliases(Journal.Add)", "Help.Aliases() -g" )
				.Group("Base"),

				Cmd("Tree")
				.Aliases( "t", "map", "rootmap" )
				.Exec(HelpTree)
				.Args(StrArg("Command Name", ArgMode::Optional))
				.Desc("Prints command tree, or command tree descended from a command if specified. Visualisation of command map/hierarchy.")
				.Examples( "Help.Tree", "Help.map()", "Help.Tree(Diary.Add)" )
				.Group("Base")

			),

			Cmd("CommandList")
			.Aliases("cmd", "commands", "command")
			.Exec(CommandList)
			.Args(StrArg("Search Key", ArgMode::Optional, ""))
			.Desc("Lists all commands, or all commands beginning with seach key if specified.")			
			.Examples( "CommandList", "cmd", "CommandList(Diary)" )
			.Group("Base")
			.Children(

				Cmd("Aliases")
				.Aliases("a", "als", "alias")
				.Exec(HelpAlias)
				.Args(StrArg("Search Key", ArgMode::Optional))
				.Options("-g")
				.Desc("Lists all aliases and corresponding canonical names for all commands, or for all commands and aliases starting with Search Key is specified.")
				.LongDesc("Lists all aliases and their canonical names for all commands, or for all commands and aliases starting with Search Key is specified. Behaviour altered with option : \n * '-g' ('group') : Prints by group.")
				.Examples( "CommandList.Aliases", "CommandList.als", "CommandList.Aliases(Journal.Add)", "CommandList.Aliases() -g" )
				.Group("Base")

			),

			Cmd("TestInput")
			.Aliases("testcommand", "tstinp")
			.Exec(TestInput)
			.Args(StrArg("Command Input", ArgMode::RequiredPrompt))
			.Options("-r")
			.Desc("Runs the TestAsync method on specified command with specified arguments. Prompts if no input is given.")
			.LongDesc("Runs the TestAsync method on specified command with specified arguments. Prompts if no input is given. Behaviour altered by options:\n * '-r' ('run'): Runs the command on success.")
			.Examples("Test({Diary.Add(This is a test entry) -f})")
			.Group("Base"),

			Cmd("Script")
			.Aliases("scrpt", "scpt")
			.Desc("Commands for running scripts by file path, nodal.")
			.Group("Base")
			.Children(
				
				Cmd("Run")
				.Aliases("r", "execute", "testandrun")
				.Exec(Script)
				.Args(StrArg("Script file path", ArgMode::RequiredPrompt, "", "Enter filepath (to cancel, leave blank, or type one of the following: { '\\', '_', 'cancel' }):", "", {"\\", "_", "", "cancel"}))
				.Options("-f")
				.Mode(1)
				.Desc("Parses, tests, and runs a script.")
				.LongDesc("Parses, tests, and runs a script. When directly passing filepath as an argument instead of at prompt, pass as raw text (without quotes ('\"') or brackets ({})), or repeat every backslash ('\\'->'\\\\'), otherwise the parser will ignore them.\nAt prompt, you can cancel the operation by leaving it blank, or typing one of the following: { '\\', '_', 'cancel' }.\nBehaviour can be altered by options:\n * '-f' ('force'): Runs the script without testing.")
				.Group("Base"),

				Cmd("Test")
				.Aliases("t", "tst")
				.Exec(Script)
				.Args(StrArg("Script file path", ArgMode::RequiredPrompt, "", "Enter filepath (to cancel, leave blank, or type one of the following: { '\\', '_', 'cancel' }):", "", { "\\", "_", "", "cancel" }))
				.Desc("Parses and tests a script.")
				.LongDesc("Parses and tests a script. When directly passing filepath as an argument instead of at prompt, pass as raw text (without quotes ('\"') or brackets ({})), or repeat every backslash ('\\'->'\\\\'), otherwise the parser will ignore them.\nAt prompt, you can cancel the operation by leaving it blank, or typing one of the following: { '\\', '_', 'cancel' }.")
				.Group("Base")

			),

			Cmd("Clear")
			.Aliases( "clr", "clearscreen" )
			.Exec(Clear)
			.Args(StrArg("Clear Message", ArgMode::Optional))
			.Options("-b")
			.Desc("Clears the screen, replacing it with optional message, or 'Cleared screen' by default.\nOption '-b' will make it blank, showing no message.")
			.Examples( "Clear", "clr", "Clear() -b" )
			.Group("Base"),

			Cmd("Echo")
			.Aliases( "ech", "print", "write" )
			.Exec(Echo)
			.Args(
				StrArg("Message", ArgMode::RequiredPrompt),
				SztArg("n", ArgMode::Optional, 1)
			)
			.Desc("Echos specified text n times (default = 1).")
			.Examples( "Echo(Hello World!)", "Echo", "print(Hello World!)", "Echo({Hello World!\\n}, 1000)" )
			.Group("Base"),

			Cmd("Exit")
			.Aliases("ext", "quit", "close", "closeapp")
			.Exec(Exit)
			.Desc("Closes the program.")
			.Examples( "Exit", "ext" )
			.Group("Base")

		);
	}

}