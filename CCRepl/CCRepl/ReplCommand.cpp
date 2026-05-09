#include "pch.h"
#include "ReplCommand.h"

namespace CCRepl {

	bool ReplCommand::CanExecute() const { return static_cast<bool>(Execute); }

	// Prints Address ane number of children.
	std::string ReplCommand::PrintRef() const {
		return std::format("{} ({} {})", Address, Children.size(), Children.size() == 1 ? "child" : "children");
	}

	std::vector<std::string> ReplCommand::GetChildAddresses() const {
		std::vector<std::string> r;
		for (ReplCommand* cmd : Children) r.push_back(cmd->Address);
		return r;
	}

	std::string ReplCommand::PrintFull() const {
		std::ostringstream oss;
		oss << Address;
		if (Group) oss << " (" << *Group << ")";
		oss << "\n";
		if (!Aliases.empty()) oss << str::PresentList(Aliases, "Aliases: ") << "\n";
		if (Usage) oss << "Usage: " << *Usage << "\n";
		if (Desc) oss << "Description: " << *Desc << "\n";
		if (!Examples.empty()) oss << str::PresentList(Examples, "Examples:", "\n    ", "\n    ", "\n");
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