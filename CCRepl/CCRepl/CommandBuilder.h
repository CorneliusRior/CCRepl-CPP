#pragma once
#include "ReplCommand.h"
#include "Parsers.h"

namespace CCRepl {

	class ReplCommand;

	// Struct used to define arguments.
	template<typename T>
	struct CmdArg {

		std::string name;
		typename ArgSpec<T>::Parser parser;
		ArgMode mode = ArgMode::Required;
		std::optional<T> fallback = std::nullopt;
		PromptInfo pmtInfo;
		std::optional<std::string> typeString;

		CmdArg(
			std::string name,
			typename ArgSpec<T>::Parser parser,
			ArgMode mode = ArgMode::Required,
			std::optional<T> fallback = std::nullopt,
			std::string prompt = "",
			std::string retryPrompt = "",
			std::vector<std::string> cancelStrings = { "\\" },
			std::optional<std::string> typeString = std::nullopt
		) : 
			name(name), parser(parser), mode(mode), fallback(fallback), typeString(typeString) {
			pmtInfo = PromptInfo{ prompt, retryPrompt, cancelStrings };
		}

	};

	// Some commands for specific data types:

	CmdArg<int> IntArg(std::string name, ArgMode mode = ArgMode::Required, std::optional<int> fallback = std::nullopt, std::string prompt = "", std::string retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" });
	CmdArg<double> DblArg(std::string name, ArgMode mode = ArgMode::Required, std::optional<double> fallback = std::nullopt, std::string prompt = "", std::string retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" });
	CmdArg<std::size_t> SztArg(std::string name, ArgMode mode = ArgMode::Required, std::optional<std::size_t> fallback = std::nullopt, std::string prompt = "", std::string retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" });
	CmdArg<std::string> StrArg(std::string name, ArgMode mode = ArgMode::Required, std::optional<std::string> fallback = std::nullopt, std::string prompt = "", std::string retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" });
	CmdArg<std::tm> DtmArg(std::string name, ArgMode mode = ArgMode::Required, std::optional<std::tm> fallback = std::nullopt, std::string prompt = "", std::string retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" });

	class CommandBuilder {
	private:

		ReplCommand _cmd;
		ReplCommand ToCmd(ReplCommand cmd) { return std::move(cmd); }
		ReplCommand ToCmd(CommandBuilder& builder) { return builder.Build(); }

	public:

		explicit CommandBuilder(std::string name);
		CommandBuilder& Exec(std::function<void(ReplContext&, CommandArgs&)> exec);
		CommandBuilder& Test(std::function<bool(ReplContext&, CommandArgs&)> test);
		CommandBuilder& Mode(int mode);
		CommandBuilder& Desc(std::string desc);
		CommandBuilder& LongDesc(std::string longDesc);
		CommandBuilder& Group(std::string group);

		template<typename T>
		void AddArg(CmdArg<T>&& spec) {
			_cmd.ArgSpecs.push_back(
				std::make_unique<ArgSpec<T>>(
					std::move(spec.name),
					spec.mode,
					std::move(spec.parser),
					std::move(spec.fallback),
					std::move(spec.pmtInfo),
					std::move(spec.typeString)
				)
			);
		}

		template<typename... Als>
		CommandBuilder& Aliases(Als&&... aliases) {
			(_cmd.Aliases.push_back(std::forward<Als>(aliases)), ...);
			return *this;
		}

		template<typename... Specs>
		CommandBuilder& Args(Specs&&... specs) {
			(AddArg(std::forward<Specs>(specs)), ...);
			return *this;
		}

		template<typename... Opt>
		CommandBuilder& Options(Opt&&... options) {
			(_cmd.Options.push_back(std::forward<Opt>(options)), ...);
			return *this;
		}

		template<typename... Exp>
		CommandBuilder& Examples(Exp&&... examples) {
			(_cmd.Examples.push_back(std::forward<Exp>(examples)), ...);
			return *this;
		}

		template<typename... Cmds>
		CommandBuilder& Children(Cmds&&... cmds) {
			(_cmd.ChildrenInit.push_back(ToCmd(std::forward<Cmds>(cmds))), ...);
			return *this;
		};

		ReplCommand Build();

	};

	CommandBuilder Cmd(std::string name);
}