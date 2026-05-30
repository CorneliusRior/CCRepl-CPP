#include "pch.h"
#include "ScptCommands.h"

namespace CCRepl {

	ScptCommands::ScptCommands() {
		Define(

			Cmd("Script")
			.Aliases("scrpt", "scpt", "scp")
			.Desc("Commands for using ScriptService, nodal.")
			.LongDesc(R"(ScriptService is a static library for loading, storing, and running .ccr scripts.)")
			.Group("Script")
			.Children(

				Cmd("ListDir")
				.Aliases("ld", "lstdir", "ListDirectory", "files", "listfiles")
				.Exec(ScpSvcList)
				.Mode(1)
				.Args(StrArg("Source", ArgMode::Optional))
				.Desc("Lists all files in the script directory, or relative path to a different directory is specified."),

				Cmd("ListLoaded")
				.Aliases("ll", "lstl", "lstloaded", "lst", "lstscripts")
				.Exec(ScpSvcList)
				.Args(StrArg("Search Key", ArgMode::Optional, ""))
				.Mode(2)
				.Desc("Lists all loaded scripts by script service, or all loaded scripts starting with search key if specified."),

				Cmd("Load")
				.Aliases("l", "ld", "import", "AddScript", "Add", "AddScp", "+")
				.Exec(ScpSvcData)
				.Mode(1)
				.Args(StrArg("FilePath", ArgMode::RequiredPrompt))
				.Options("-a", "-q")
				.Desc("Loads and parses a script to script service.")
				.LongDesc("Loads and parses a script and saves it in script service. By default, will search in the /scripts/ folder for a file with the same name. Can pass the absolute path with option '-a' ('absolute'). When directly passing filepath as an argument instead of at prompt, pass as raw text (without quotes ('\"') or brackets ({})), or repeat every backslash ('\\'->'\\\\'), otherwise the parser will ignore them.\nBehaviour can be altered by options:\n * '-a' ('absolute'): Takes 'FilePath' to be absolute path instead of relative path.\n * '-q' ('quiet'): Prints less information.")
				.Examples("Script.ScriptService.Load(Script1.txt)", "scpt.s.l(C:\\Users\\User\\Desktop\\Script2.txt) -a")
				.Children(

					Cmd("Dir")
					.Aliases("d", "a", "all", "*", "directory")
					.Exec(ScpSvcData)
					.Mode(2)
					.Args(StrArg("Source", ArgMode::Optional))
					.Options("-a", "-q")
					.Desc("Loads and parses every script in a directory (/scripts/ by default).")
					.LongDesc("Loads and parses every script in a directory. By default, this is every script in /scripts/. 'Source' argument is a path relative to /scripts/, unless '-a' option is used.\nBehaviour can be altered by options:\n * '-a' ('absolute'): Takes 'Source' to be absolute path instead of relative path.\n * '-q' ('quiet'): Prints less information.")

				),

				Cmd("Unload")
				.Aliases("u", "rm", "delete", "del", "-")
				.Exec(ScpSvcData)
				.Mode(3)
				.Args(StrArg("Script name", ArgMode::RequiredPrompt))
				.Options("-f", "-q")
				.Desc("Unloads a given script in script service. Prompts with y/n confirmation.")
				.LongDesc("Unloads a given script in script service. Prompts with y/n confirmation. Behaviours can be altered by options:\n * '-f' ('force'): Skips y/n confirmatin.\n * '-q' ('quiet'): Prints less information.")
				.Children(

					Cmd("All")
					.Aliases("d", "dir", "a", "all", "*", "directory")
					.Exec(ScpSvcData)
					.Mode(4)
					.Options("-f", "-q", "-s")
					.Desc("Unloads every script in script service.")
					.LongDesc("Unloads every script in script service. Prompts with y/n confirmation. Behaviours can be altered by options:\n * '-f' ('force'): Skips y/n confirmation.\n * '-q' ('quiet'): Prints less information.\n * '-s' ('select'): Iterates through each loaded script and prompted to delete.")
					.Options()

				),

				Cmd("Rename")
				.Aliases("rnm", "rn")
				.Exec(ScpSvcData)
				.Mode(5)
				.Args(
					StrArg("OldScriptName", ArgMode::RequiredPrompt),
					StrArg("NewScriptName", ArgMode::RequiredPrompt)
				)
				.Options("-f", "-q")
				.Desc("Rename a loaded file.")
				.LongDesc("Rename a loaded file. Behaviour can be altered by options:\n * '-f' ('force'): Bypasses Y/N confirmation.\n * 'q' ('quiet'): Does not print script list afterwards."),

				Cmd("Run")
				.Aliases("r", "execute", "ex", "testandrun")
				.Exec(ScpSvcRun)
				.Mode(1)
				.Args(StrArg("ScriptName", ArgMode::RequiredPrompt))
				.Options("-f", "-p")
				.Desc("Tests and runs a loaded script.")
				.LongDesc("Tests and runs a loaded script.Behaviour can be altered by options:\n * '-f' ('force'): Runs the script without testing.\n * '-p' ('print'): Prints more information."),

				Cmd("Test")
				.Aliases("t", "tst")
				.Exec(ScpSvcRun)
				.Args(StrArg("ScriptName", ArgMode::RequiredPrompt))
				.Options("-p")
				.Desc("Tests a loaded script.")
				.LongDesc("Tests a loaded script. Behaviour can be altered by options:\n * '-p' ('print'): Prints more information."),

				Cmd("Print")
				.Aliases("p", "pnt", "printinfo", "info")
				.Exec(ScpSvcPrint)
				.Args(StrArg("ScriptName", ArgMode::RequiredPrompt))
				.Desc("Prints info on a loaded script.")

			)

		);
	}

	// Listing
	CMD_H(ScpSvcList) {
		CCSS_SET_SVC();
		switch (args.Mode()) {
		case 1: {
			std::optional<std::string> dir = args.Get<std::string>(0);
			if (dir) ctx.WriteLine(svc->ListDir(*dir));
			else ctx.WriteLine(svc->ListDir());
			break;
		}
		case 2: {
			ctx.WriteLine(svc->ListScripts(args.GetR<std::string>(0)));
			break;
		}
		default:
			throw ReplException("Unknown command mode: " + args.Mode());
		}
	}

	// Loading & unloading
	CMD_H(ScpSvcData) {
		CCSS_SET_SVC();

		switch (args.Mode()) {

		// Load one
		case 1: {
			std::string fileName = args.GetR<std::string>(0);
			if (args.HasOptStart("-a")) CTX_WAIT_SPIN(bool, svc->LoadScriptAbs(fileName), std::format("Loading file '{}'", fileName), "Loaded.");
			else CTX_WAIT_SPIN(bool, svc->LoadScript(fileName), std::format("Loading file '{}'", fileName), "Loaded.");
			if (!args.HasOptStart("-q")) ctx.WriteLine(svc->ListScripts(""));
			break;
		}

		// Load all
		case 2: {
			if (args.HasOptStart("-a")) CTX_WAIT_SPIN(bool, svc->LoadAllAbs(args.GetR<std::string>(0)), "Loading", "Loaded.");
			else CTX_WAIT_SPIN(bool, svc->LoadAll(args.GetOr<std::string>(0, "")), "Loading", "Loaded.");
			if (!args.HasOptStart("-q")) ctx.WriteLine(svc->ListScripts(""));
			break;
		}

		// Unload one
		case 3: {
			std::string scpName = args.GetR<std::string>(0);
			if (svc->HasScript(scpName)) {
				if (args.HasOptStart("-f") || ctx.Confirm(std::format("Delete script '{}'? (Y/N): ", scpName))) {
					if (svc->Unload(scpName)) if (!args.HasOptStart("-q")) ctx.WriteLine(std::format("Deleted script '{}'", scpName));
					else throw ReplException("No script deleted.");
				}
				else ctx.WriteLine("Cancelled.");
			}
			else ctx.WriteLine(std::format("No script with name '{}'", scpName));
			break;
		}

		// Unload all
		case 4: {
			if (args.HasOptStart("-s")) {
				for (std::string scp : svc->ScriptNames()) {
					if (ctx.Confirm(std::format("Unload script '{}'? (Y/N): ", scp))) svc->Unload(scp);
				}
				if (!args.HasOptStart("-q")) ctx.WriteLine(svc->ListScripts(""));
			}
			else {
				if (args.HasOptStart("-f") || ctx.Confirm("Delete all scripts? (Y/N): ")) svc->UnloadAll();
				else ctx.WriteLine("Cancelled.");
			}
			break;
		}

		// Rename
		case 5: {
			std::string oldName = args.GetR<std::string>(0);
			std::string newName = args.GetR<std::string>(1);
			svc->RenameScript(oldName, newName);
			if (!args.HasOptStart("-q")) {
				ctx.WriteLine(std::format("Renamed '{}' to '{}'", oldName, newName));
				ctx.WriteLine(svc->ListScripts(""));
			}
			break;
		}

		default: throw ReplException(std::format("Unknown Command mode: {}", args.Mode()));

		}
	}

	// Testing/running
	CMD_H(ScpSvcRun) {
		std::string name = args.GetR<std::string>(0);
		CCSS_SET_SVC();
		if (!svc->HasScript(name)) throw ReplUserException(std::format("No loaded script with name '{}'", name));
		if (args.IsMode(1)) {
			if (args.HasOptStart("-f")) svc->ExecuteScript(name);
			else {
				if (svc->TestScript(name, !args.HasOptStart("-p"))) {
					ctx.WriteLine("Running...");
					svc->ExecuteScript(name);
				}
				else ctx.WriteLine("Test failed, not running. Use option '-f' to force run if you believe this is a mistake.");
			}
		}
		else svc->TestScript(name, !args.HasOptStart("-p"));
	}

	CMD_H(ScpSvcPrint) {
		ctx.WriteLine(CCSS_GET_SVC()->PrintScript(args.GetR<std::string>(0)));
	}

}