#include "pch.h"
#include "CommandBuilder.h"

namespace CCRepl {

	CmdArg<int> IntArg(std::string name, ArgMode mode, std::optional<int> fallback, std::string prompt, std::string retryPrompt, std::vector<std::string> cancelStrings) {
		return CmdArg<int>(name, parsers::TryInt, mode, fallback, prompt, retryPrompt, cancelStrings);
	}

	CmdArg<double> DblArg(std::string name, ArgMode mode, std::optional<double> fallback, std::string prompt, std::string retryPrompt, std::vector<std::string> cancelStrings) {
		return CmdArg<double>(name, parsers::TryDouble, mode, fallback, prompt, retryPrompt, cancelStrings);
	}

	CmdArg<std::size_t> SztArg(std::string name, ArgMode mode, std::optional<std::size_t> fallback, std::string prompt, std::string retryPrompt, std::vector<std::string> cancelStrings) {
		return CmdArg<std::size_t>(name, parsers::TrySize_t, mode, fallback, prompt, retryPrompt, cancelStrings);
	}

	CmdArg<std::string> StrArg(std::string name, ArgMode mode, std::optional<std::string> fallback, std::string prompt, std::string retryPrompt, std::vector<std::string> cancelStrings) {
		return CmdArg<std::string>(name, parsers::TryString, mode, fallback, prompt, retryPrompt, cancelStrings);
	}

	CmdArg<std::tm> DtmArg(std::string name, ArgMode mode, std::optional<std::tm> fallback, std::string prompt, std::string retryPrompt, std::vector<std::string> cancelStrings) {
		return CmdArg<std::tm>(name, parsers::TryTime, mode, fallback, prompt, retryPrompt, cancelStrings);
	}

	CommandBuilder::CommandBuilder(std::string name) { _cmd.Name = std::move(name); }

	CommandBuilder& CommandBuilder::Exec(std::function<void(ReplContext&, CommandArgs&)> newExec) { _cmd.Execute = newExec; return *this; }
	//CommandBuilder& CommandBuilder::Options(std::vector<std::string> options) { _cmd.Options = options; return *this; }
	CommandBuilder& CommandBuilder::Mode(int mode) { _cmd.Mode = mode; return *this; }
	CommandBuilder& CommandBuilder::Desc(std::string desc) { _cmd.Desc = desc; return *this; }
	CommandBuilder& CommandBuilder::LongDesc(std::string longDesc) { _cmd.LongDesc = longDesc; return *this; }
	CommandBuilder& CommandBuilder::Group(std::string group) { _cmd.Group = group; return *this; }
	CommandBuilder& CommandBuilder::Examples(std::vector<std::string> examples) { _cmd.Examples = examples; return *this; }

	ReplCommand CommandBuilder::Build() { return std::move(_cmd); }

	CommandBuilder Cmd(std::string name) { return CommandBuilder(move(name)); }


}