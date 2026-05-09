#include "pch.h"
#include "CommandArgs.h"
#include "ReplContext.h"

namespace CCRepl {

	CommandArgs::CommandArgs(ReplContext& ctx, ReplCommand* cmd, const std::vector<std::string> args, const std::vector<std::string> opt) {

		CommandAddress = cmd->Address;
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

	bool CommandArgs::HasOption(const std::string& opt) { return str::InVector(opt, Options); }
}