#pragma once

#include "CommandArgs.h"

namespace CCRepl {

	class CommandArgs;
	class ReplContext;

	enum class HelpAttribute {
		Aliases,
		Description,
		Examples,
		Full,
		LongDescription,
		Usage
	};

	std::string ToString(HelpAttribute help);

	class ReplCommand {
	public:

		std::string Name;
		std::string Address;
		std::vector<std::string> Aliases;
		std::function<void(ReplContext&, CommandArgs&)> Execute;
		std::function<bool(ReplContext&, CommandArgs&)> Test;
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
		bool CanTest() const;

		// Output/Print:
		std::string PrintRef() const;
		std::string PrintFull() const;
		std::string PrintIndexLine(HelpAttribute help, std::size_t col, std::size_t total, bool oneline = true) const;
		std::vector<std::string> GetChildAddresses() const;
		std::string PrintTree(std::string namePrefix, std::string listPrefix);
	};

}