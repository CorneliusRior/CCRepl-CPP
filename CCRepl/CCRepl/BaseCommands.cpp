#include "pch.h"
#include <ranges>
#include <set>
#include "CommandBuilder.h"
#include "CommandSet.h"
#include "fmt.h"
#include "ReplCommand.h"
#include "ReplContext.h"
#include "Script.h"

namespace CCRepl {

	CMD_H(Handler) {
		ctx.WriteLine("Not yet implemented.");
	}

	CMD_H(About) {
		ctx.WriteLine(ctx.AboutStr);
	}

	CMD_H(Help) {

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
		int attMode = args.FirstOptionStart("-a", "-d", "-e", "-f", "-i", "-l", "-u");
		switch (attMode) {
		case 0:		att = HelpAttribute::Aliases;			 olDefault = true;	break;
		case 1:		att = HelpAttribute::Description;		 olDefault = true;	break;
		case 2:		att = HelpAttribute::Examples;			 olDefault = false;	break;
		case 3:		att = HelpAttribute::Full;				 olDefault = false;	break;
		case 4:		att = HelpAttribute::Implemented;		 olDefault = true;	break;
		case 5:		att = HelpAttribute::LongDescription;	 olDefault = false;	break;
		case 6:		att = HelpAttribute::Usage;				 olDefault = true;	break;
		default:	
			att = inputKey.has_value() ? HelpAttribute::Full : HelpAttribute::Description;
			olDefault = true; 
			break;
		}

		bool group = args.HasOptStart("-g");

		// Define col here w/ max:
		std::size_t col = 0;
		for (const auto& it : filtered) if (it.first.size() > col) col = it.first.size();
		col += 5;
		std::size_t total = 180 - col;
		std::ostringstream oss;

		// Write it out:
		if (group) {
			std::set<std::string> seen;
			std::vector<std::string> groups;
			for (const auto& it : filtered) {
				std::string group = it.second.Group;
				if (seen.insert(group).second) groups.push_back(group);
			}

			// Sort groups:
			std::sort(groups.begin(), groups.end(),
				[](const auto& a, const auto& b) {
					auto rank = [](const auto& s) {
						if (s == "Base") return 0;
						if (s == "Ungrouped") return 2;
						return 1;
						};
					int ra = rank(a);
					int rb = rank(b);
					if (ra != rb) return ra < rb;
					return a < b;
				});

			for (std::string g : groups) {
				auto gfiltered = ctx.CommandReg | std::views::filter(
					[&sk, &g](const auto& it) { return it.first.starts_with(sk) && it.second.Group == g;}
				);

				// Print banner:
				oss << "\n * " << g << " (" << std::ranges::distance(gfiltered) << " total):\n";

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
		std::string beginStmt = inputKey.has_value() ? std::format(" beginning with '{}'", ik) : "";
		if (att == HelpAttribute::Full) ctx.WriteLine(std::format("Printing full information for all commands{} ({} total):\n", beginStmt, count));
		else ctx.WriteLine(std::format("Printing addresses and {} for all commands{} ({} total):\n", ToString(att), beginStmt, count));

		ctx.WriteLine(oss.str());
		// Test when you get back.
	}

	// Silly test, frankly you can disregard this one:
	CMD_T(HelpTest) {
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

	CMD_H(HelpAlias) {

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
			std::set<std::string> seen;
			std::vector<std::string> groups;
			for (const auto& it : filtered) {
				std::string group = it.second->Group;
				if (seen.insert(group).second) groups.push_back(group);
			}

			// Sort groups:
			std::sort(groups.begin(), groups.end(),
				[](const auto& a, const auto& b) {
					auto rank = [](const auto& s) {
						if (s == "Base") return 0;
						if (s == "Ungrouped") return 2;
						return 1;
						};
					int ra = rank(a);
					int rb = rank(b);
					if (ra != rb) return ra < rb;
					return a < b;
				});

			// Add each to string:
			for (std::string g : groups) {
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
				oss << "\n * " << g << " (" << std::ranges::distance(gfiltered) << " total aliases for " << gcount << " commands) :\n";

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

	CMD_H(HelpTree) {
		std::optional<std::string> inputCmd = args.Get<std::string>(0);
		HelpAttribute att;
		int attMode = args.FirstOptionStart("-a", "-d", "-e", "-i", "-l", "-u");
		switch (attMode) {
		case 0: att = HelpAttribute::Aliases;			break;
		case 1: att = HelpAttribute::Description;		break;
		case 2: att = HelpAttribute::Examples;			break;
		case 3: att = HelpAttribute::Implemented;		break;
		case 4: att = HelpAttribute::LongDescription;	break;
		case 5: att = HelpAttribute::Usage;				break;
		default: att = HelpAttribute::None;				break;
		}

		if (!inputCmd) ctx.WriteLine(ctx.RootTree(att));
		else {
			ReplCommand* cmd = ctx.FindCommand(str::DotSeparated(*inputCmd));
			std::size_t col = str::MaxLength(cmd->PrintTree("", "")) + 5;
			ctx.WriteLine(cmd->PrintTree("", "", att, col));
		}
	}

	CMD_H(CommandList) {
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

	CMD_H(TestInput) {
		std::string input = args.GetRequired<std::string>(0);
		ctx.Test(input, args.HasOptStart("-r"));
	}

	CMD_H(ScriptHandler) {
		// just occured to me that "mode" should be in commandargs instead of ctx, will change that.
		bool PrintAll = args.HasOptStart("-p");

		std::string path = args.GetRequired<std::string>(0);
		std::string scriptTxt = CTX_WAIT_SPIN(std::string, str::ReadTextFile(path), std::format("Loading file '{}'", path), "Loaded.");
		if (PrintAll) ctx.WriteLine(scriptTxt + "\n\n");
		Script scp = CTX_WAIT_SPIN(Script, TextToScript(ctx, scriptTxt), "Generating Script", "Generated.");

		if (args.IsMode(1)) {
			if (args.HasOptStart("-f")) scp.Execute(ctx);
			else {
				if (scp.Test(ctx, !PrintAll)) {
					ctx.WriteLine("Running...");
					scp.Execute(ctx);
				}
				else ctx.WriteLine("Test failed, not running. Use option '-f' to force run if you believe this is a mistake.");
			}
		}
		else scp.Test(ctx, !PrintAll);
	}

	CMD_H(Clear) {
		if (args.Opt("-b")) ctx.Clear();
		else ctx.Clear(args.GetOr<std::string>(0, "Cleared Screen."));
	}

	CMD_H(Echo) {
		std::string r = args.GetRequired<std::string>(0);
		for (std::size_t i = 0; i < args.GetRequired<std::size_t>(1); i++) {
			ctx.Write(r);
		}
		ctx.WriteLine();
	}

	CMD_H(Exit) {
		ctx.CloseApp();
	}

	BaseCommands::BaseCommands() {
		Define(

			Cmd("About")
			.Aliases("abt", "info")
			.Exec(About)
			.Desc("Prints the 'about' statement.")
			.LongDesc("Prints the 'about' statement, which can be set for each program.\nTo set the 'about' statement, assign the desired string to 'ReplContext::AboutStr' after construction.")
			.Group("Base"),

			Cmd("Help")
			.Aliases("h", "?")
			.Exec(Help)
			.Test(HelpTest)
			.Args(StrArg("Search Key", ArgMode::Optional))
			.Options("-a", "-d", "-e", "-f", "-g", "-i", "-l", "-m", "-o", "-u")
			.Desc("Lists all commands and descriptions, or shows full help for all commands with Search Key is specified.")
			.LongDesc(
				R"(Lists all commands and descriptions, or full help for all commands starting with Search Key. Behaviour altered with options:
 * '-a' ('aliases'): Prints list of all aliases for that command node (to see full list of all possible combinations, see Help.Alias).
 * '-d' ('description'): Prints full description without truncation.
 * '-e' ('example'): Prints example usages.
 * '-f' ('full'): Prints full info regardless of search key presence.
 * '-g' ('group'): Prints by group (by default only done with no search term. Use '-o' to ungroup that)
 * '-i' ('implemented'): Gives [x] if command is marked as implemented, otherwise [ ] to indicate work in progress.
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
				.Examples( "Help.Aliases", "Help.als", "Help.Aliases(Journal.Add)", "Help.Aliases() -g" ),

				Cmd("Tree")
				.Aliases( "t", "map", "rootmap" )
				.Exec(HelpTree)
				.Args(StrArg("Command Name", ArgMode::Optional))
				.Options("-a", "-d", "-e", "-i", "-l", "-u")
				.Desc("Prints command tree, or command tree descended from a command if specified. Visualisation of command map/hierarchy.")
				.LongDesc(
					R"(Prints command tree, or command tree descended from a command if specified. Visualisation of command map/hierarchy. Behaviour altered with options:
 * '-a' ('aliases'): Prints lists of aliases.
 * '-d' ('description'): Prints description.
 * '-e' ('example'): Prints an example usage.
 * '-i' ('implemented'): Gives [x] if command is marked as implemented, otherwise [ ] to indicate work in progress.
 * '-l' ('longdescription'): Prints first line of long description.
 * '-u' ('usage'): Prints usage statement.)"
				)
				.Examples( "Help.Tree", "Help.map()", "Help.Tree(Diary.Add)" )

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
				.Aliases("r", "execute", "ex", "testandrun")
				.Exec(ScriptHandler)
				.Args(StrArg("Script file path", ArgMode::RequiredPrompt, "", "Enter filepath (to cancel, leave blank, or type one of the following: { '\\', '_', 'cancel' }):", "", {"\\", "_", "", "cancel"}))
				.Options("-f", "-p")
				.Mode(1)
				.Desc("Parses, tests, and runs a script by file path.")
				.LongDesc("Parses, tests, and runs a script by file path. When directly passing filepath as an argument instead of at prompt, pass as raw text (without quotes ('\"') or brackets ({})), or repeat every backslash ('\\'->'\\\\'), otherwise the parser will ignore them.\nAt prompt, you can cancel the operation by leaving it blank, or typing one of the following: { '\\', '_', 'cancel' }.\nBehaviour can be altered by options:\n * '-f' ('force'): Runs the script without testing.\n * '-p' ('print'): Prints more information when parsing.")
				.Examples( "Script.Run()", "Script.Run(C:\\Users\\User\\Desktop\\Script.txt)", "Script.Run() -f"),

				Cmd("Test")
				.Aliases("t", "tst")
				.Exec(ScriptHandler)
				.Args(StrArg("Script file path", ArgMode::RequiredPrompt, "", "Enter filepath (to cancel, leave blank, or type one of the following: { '\\', '_', 'cancel' }):", "", { "\\", "_", "", "cancel" }))
				.Options("-p")
				.Desc("Parses and tests a script by file path.")
				.LongDesc("Parses and tests a script by file path. When directly passing filepath as an argument instead of at prompt, pass as raw text (without quotes ('\"') or brackets ({})), or repeat every backslash ('\\'->'\\\\'), otherwise the parser will ignore them.\nAt prompt, you can cancel the operation by leaving it blank, or typing one of the following: { '\\', '_', 'cancel' }.\nBehaviour can be altered by options:\n * '-p' ('print'): Prints more information when parsing.")
				.Examples("Script.Test()", "Script.Test(C:\\Users\\User\\Desktop\\Script.txt)")
				.Group("Test")
				.WIP()

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