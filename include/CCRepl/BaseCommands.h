#pragma once
#include <CCRepl/CommandSet.h>

namespace CCRepl {

	class BaseCommands : public CommandSet {
	public:
		BaseCommands();
	};

	CMD_H(About);
	CMD_H(Help);
	CMD_H(HelpAlias);
	CMD_H(HelpTree);
	CMD_H(CommandList);
	CMD_H(TestInput);
	CMD_H(ScriptHandler);
	CMD_H(Clear);
	CMD_H(Echo);
	CMD_H(Exit);

}