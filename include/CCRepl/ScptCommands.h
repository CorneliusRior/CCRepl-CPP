#pragma once
#include <CCRepl/CommandSet.h>
#include <CCRepl/ScriptService.h>

namespace CCRepl {

	class ScptCommands : public CommandSet {
	public:
		ScptCommands();
	};

	CMD_H(ScpSvcList);
	CMD_H(ScpSvcData);
	CMD_H(ScpSvcRun);
	CMD_H(ScpSvcPrint);

}