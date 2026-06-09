#pragma once
#include <CCRepl/CommandArgs.h>
#include <util/fmt.h>

#define CMD_H(name) static void name(CCRepl::ReplContext& ctx, CCRepl::CommandArgs& args)
#define CMD_T(name) static bool name(CCRepl::ReplContext& ctx, CCRepl::CommandArgs& args)

namespace CCRepl {

	class CommandArgs;
	class ReplContext;

	enum class HelpAttribute {
		None,
		Aliases,
		Description,
		Examples,
		Full,
		Implemented,
		LongDescription,
		Usage
	};

	std::string ToString(HelpAttribute help);

	class ReplCommand {
	public:

		ReplCommand* Parent = nullptr;	// null for root commands.
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
		std::string Group = "";
		std::vector<std::string> Options;
		std::vector<std::string> Examples;
		std::vector<ReplCommand> ChildrenInit;
		std::vector<ReplCommand*> Children;
		bool Implemented = true;
		
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
		std::string PrintFull(std::size_t max) const;
		std::string PrintIndexLine(HelpAttribute help, std::size_t col, std::size_t total, bool oneline = true) const;
		std::vector<std::string> GetChildAddresses() const;
		std::string PrintTree(std::string namePrefix, std::string listPrefix, HelpAttribute help = HelpAttribute::None, std::size_t col =  50, std::size_t total = 180);
	};

}