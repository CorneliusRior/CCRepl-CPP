#pragma once

#include "CommandArgs.h"

namespace CCRepl {

	class CommandArgs;
	class ReplContext;

	class ReplCommand {
	public:

		std::string Name;
		std::string Address;
		std::vector<std::string> Aliases;
		std::function<void(ReplContext&, CommandArgs&)> Execute;
		std::vector<std::unique_ptr<IArgSpec>> ArgSpecs;
		std::optional<int> Mode;
		std::optional<std::string> Desc;
		std::optional<std::string> LongDesc;
		std::optional<std::string> Usage;
		std::optional<std::string> Group;
		std::vector<std::string> Options;
		std::vector<std::string> Examples;
		std::vector<ReplCommand> ChildrenInit;
		std::vector<ReplCommand*> Children;
		
		// Points to its own listing in ReplContext.CommandReg
		ReplCommand* RegPtr = nullptr;

		ReplCommand() = default;
		ReplCommand(const ReplCommand&) = delete;
		ReplCommand& operator=(const ReplCommand&) = delete;
		ReplCommand(ReplCommand&&) noexcept = default;
		ReplCommand& operator=(ReplCommand&&) noexcept = default;

		bool CanExecute() const;

		// Output/Print:
		std::string PrintRef() const;
		std::string PrintFull() const;
		std::vector<std::string> GetChildAddresses() const;
		std::string PrintTree(std::string namePrefix, std::string listPrefix);
	};

}