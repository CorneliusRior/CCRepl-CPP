#pragma once
#include <CCRepl/ReplCommand.h>
#include <CCRepl/CommandBuilder.h>

namespace CCRepl {

	class CommandSet {
	protected:
		std::vector<ReplCommand> commands_;

		ReplCommand ToCmd(ReplCommand cmd) { return std::move(cmd); }
		ReplCommand ToCmd(CommandBuilder& builder) { return builder.Build(); }
		ReplCommand ToCmd(CommandBuilder&& builder) { return builder.Build(); }

		template<typename... Cmds>
		void Define(Cmds&&... cmds) {
			(commands_.push_back(ToCmd(std::forward<Cmds>(cmds))), ...);
		}

	public:
		virtual ~CommandSet() = default;
		virtual std::vector<ReplCommand> GetCommands() { return std::move(commands_); }
	};
	
	class TestCommands : public CommandSet {
	public:
		TestCommands();
	};

}