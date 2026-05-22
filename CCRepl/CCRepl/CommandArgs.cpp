#include "pch.h"
#include "CommandArgs.h"
#include "ReplContext.h"
#include "Script.h"

namespace CCRepl {

	std::size_t CommandArgs::GetIdByName(const std::string& name) const {
		for (std::size_t i = 0; i < Cmd->ArgSpecs.size(); i++) {
			if (Cmd->ArgSpecs[i]->Name == name) return i;
		}
		throw ReplException(std::format("No argument with name '{}'", name));
	}

	CommandArgs::CommandArgs(ReplContext& ctx, ReplCommand* cmd, const std::vector<std::string> args, const std::vector<std::string> opt) {

		Cmd = cmd;
		CommandAddress = Cmd->Address;
		Options = opt;

		// Get ArgSpecs from command, iterate through each w/ args.
		const std::vector<std::unique_ptr<IArgSpec>>& specs = cmd->ArgSpecs;
		for (std::size_t i = 0; i < specs.size(); i++) {
			const IArgSpec& spec = *specs[i];
			
			// If argument is present, parse it:
			if (i < args.size()) {
				// If it is optional and equal to a cancel string, return fallback. Otherwise, parse::
				if (!IsRequired(spec.Mode) && str::InVector(args[i], spec.PmtInfo.cancelStrings)) Args.push_back(spec.Fallback());				
				else Args.push_back(spec.Parse(args[i]));
			}

			// If argument is not present:
			else {
				switch (spec.Mode) {

				case ArgMode::Required:
					throw ReplUserException(std::format("Not enough arguments, missing argument {}.", spec.Print()));
					break;

				case ArgMode::RequiredPrompt: {
					PromptInfo info = spec.PmtInfo;
					while (true) {
						try {
							std::string input = ctx.ReadLine(info.prompt);
							if (str::InVector(input, info.cancelStrings)) throw ReplCancel(input.append(" in cancellation strings."));
							Args.push_back(spec.Parse(input));
							break;
						}
						catch (ReplUserException&) { ctx.WriteLine(info.retryPrompt); }
					}
					break;
				}

				case ArgMode::Optional:
					Args.push_back(spec.Fallback());
					break;

				case ArgMode::OptionalPrompt: {
					PromptInfo info = spec.PmtInfo;
					while (true) {
						try {
							std::string input = ctx.ReadLine(info.prompt);
							if (str::InVector(input, info.cancelStrings)) Args.push_back(spec.Fallback());
							else Args.push_back(spec.Parse(input));
							break;
						}
						catch (ReplUserException&) {
							ctx.WriteLine(info.retryPrompt);
						}
					}
					break;
				}
				}
			}
		}
	}

	CommandArgs::CommandArgs(ReplContext& ctx, const ScriptToken& tokens) {

		CommandTokens tk = tokens.tokens;
		Cmd = ctx.FindCommand(tk.commandHead);
		CommandAddress = Cmd->Address;
		Options = tk.opts;

		std::vector<std::string> args = tk.args;

		// Get ArgSpecs from command, iterate through each w/ args.
		const std::vector<std::unique_ptr<IArgSpec>>& specs = Cmd->ArgSpecs;
		for (std::size_t i = 0; i < specs.size(); i++) {
			const IArgSpec& spec = *specs[i];

			// If argument is present, parse it:
			if (i < args.size()) {
				// If it is optional and equal to a cancel string, return fallback. Otherwise, parse:
				if (!IsRequired(spec.Mode) && str::InVector(args[i], spec.PmtInfo.cancelStrings)) Args.push_back(spec.Fallback());
				else Args.push_back(spec.Parse(args[i]));
			}
			else {
				// Unlike normal input, don't prompt, fallback or throw.
				if (!IsRequired(spec.Mode)) Args.push_back(spec.Fallback());
				else throw ReplUserException(std::format("Not enough arguments, missing argument {}.", spec.Print()));
			}
		}

	}

	int CommandArgs::Mode() { return Cmd->Mode.value_or(0); }
	bool CommandArgs::IsMode(int mode) { return Mode() == mode; }
	bool CommandArgs::HasOption(const std::string& opt) { return str::InVector(opt, Options); }
	bool CommandArgs::HasOptStart(const std::string& opt) {
		for (const std::string& o : Options) if (str::StartsWith(o, opt)) return true;
		return false;
	}
}