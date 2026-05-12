#include "pch.h"
#include "ReplCommand.h"

namespace CCRepl {

	std::string ToString(HelpAttribute help) {
		switch (help) {
		case HelpAttribute::Aliases:			return "aliases";
		case HelpAttribute::Description:		return "descriptions";
		case HelpAttribute::Examples:			return "examples";
		case HelpAttribute::Full:				return "all details";
		case HelpAttribute::LongDescription:	return "long descriptions";
		case HelpAttribute::Usage:				return "usage statements";
		default: return "ERROR: Unknown HelpAttribute";
		}
	}


	bool ReplCommand::CanExecute() const { return static_cast<bool>(Execute); }
	bool ReplCommand::CanTest() const { return static_cast<bool>(Test); }

	// Prints Address ane number of children.
	std::string ReplCommand::PrintRef() const {
		return std::format("{} ({} {})", Address, Children.size(), Children.size() == 1 ? "child" : "children");
	}

	std::string ReplCommand::PrintIndexLine(HelpAttribute help, std::size_t col, std::size_t total, bool oneline) const {
		switch (help) {
		case HelpAttribute::Aliases:
			return str::ToIndexLine(Address, (Aliases.size() == 0 ? "-" : str::PresentList(Aliases)), col, total, oneline);
		case HelpAttribute::Description:
			return str::ToIndexLine(Address, Desc.value_or("-"), col, total, oneline);
		case HelpAttribute::Examples:
			return str::ToIndexLine(Address, (Examples.size() == 0 ? "-" : str::PresentList(Examples, "", "\n", "", "")), col, total, oneline);
		case HelpAttribute::Full:
			return PrintFull();
		case HelpAttribute::LongDescription:
			return str::ToIndexLine(Address, LongDesc.value_or("-"), col, total, oneline);
		case HelpAttribute::Usage:
			return str::ToIndexLine(Address, Usage.value_or("-"), col, total, oneline);
		default:
			return "ERROR: Unknown HelpAttribute.";
		}
	}

	std::vector<std::string> ReplCommand::GetChildAddresses() const {
		std::vector<std::string> r;
		for (ReplCommand* cmd : Children) r.push_back(cmd->Address);
		return r;
	}

	std::string ReplCommand::PrintFull() const {
		std::ostringstream oss;
		oss << '*' << Address;
		if (Group) oss << " (" << *Group << ")";
		oss << '\n';
		if (!Aliases.empty()) oss << str::PresentList(Aliases, "Aliases: ") << '\n';
		if (Usage) oss << "Usage: " << *Usage << '\n';
		if (Desc) oss << "Description: " << *Desc << '\n';
		if (!Examples.empty()) oss << str::PresentList(Examples, "Examples:", "\n    ", "\n    ", "\n");
		if (LongDesc) oss << *LongDesc << '\n';
		if (Children.size() > 0) {
			if (Children.size() == 1) oss << "1 child: [ " << Children[0]->Address << " ]";
			else oss << Children.size() << str::PresentList(GetChildAddresses(), " children: ");
			oss << '\n';
		}
		return oss.str();
	}

	std::string ReplCommand::PrintTree(std::string namePrefix, std::string listPrefix) {
		std::ostringstream oss;
		oss << namePrefix << '[' << Name << "]\n";
		for (size_t i = 0; i < Children.size(); i++) {
			if (i == Children.size() - 1) oss << Children[i]->PrintTree(listPrefix + " └─", listPrefix + "   ");
			else oss << Children[i]->PrintTree(listPrefix + " ├─", listPrefix + " │ ");
		}
		return oss.str();
	}
}