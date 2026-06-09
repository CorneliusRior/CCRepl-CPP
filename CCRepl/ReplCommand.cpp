#include "pch.h"
#include <CCRepl/ReplCommand.h>

namespace CCRepl {

	std::string ToString(HelpAttribute help) {
		switch (help) {
		case HelpAttribute::None:				return "none";
		case HelpAttribute::Aliases:			return "aliases";
		case HelpAttribute::Description:		return "descriptions";
		case HelpAttribute::Examples:			return "examples";
		case HelpAttribute::Implemented:		return "implementation statuses";
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

	std::string ReplCommand::PrintIndexLine(HelpAttribute help, std::size_t col, std::size_t max, bool oneline) const {
		switch (help) {
		case HelpAttribute::None:
			return Address;
		case HelpAttribute::Aliases:
			return str::ToIndexLine(Address, (Aliases.size() == 0 ? "-" : str::PresentList(Aliases)), col, max, oneline);
		case HelpAttribute::Description:
			return str::ToIndexLine(Address, Desc.value_or("-"), col, max, oneline);
		case HelpAttribute::Examples:
			return str::ToIndexLine(Address, (Examples.size() == 0 ? "-" : str::PresentList(Examples, "", "\n", "", "")), col, max, oneline);
		case HelpAttribute::Full:
			return PrintFull(max) + "\n";
		case HelpAttribute::Implemented:
			return str::ToIndexLine(Address, Implemented ? "[x]" : "[ ]", col, max, oneline);
		case HelpAttribute::LongDescription:
			return str::ToIndexLine(Address, LongDesc.value_or("-"), col, max, oneline);
		case HelpAttribute::Usage:
			return str::ToIndexLine(Address, Usage.value_or("-"), col, max, oneline);
		default:
			return "ERROR: Unknown HelpAttribute.";
		}
	}

	std::vector<std::string> ReplCommand::GetChildAddresses() const {
		std::vector<std::string> r;
		for (ReplCommand* cmd : Children) r.push_back(cmd->Address);
		return r;
	}

	std::string ReplCommand::PrintFull(std::size_t boxw) const {
		std::size_t col = 15;
		std::size_t boxhpad = 3;
		std::size_t max = boxw - col - (boxhpad * 2);

		std::ostringstream oss;

		/*oss << '*' << Address
			<< " (" << Group << ")"
			<< (Implemented ? "" : " [WIP]");
		oss << "\nParent: "
			<< (Parent ? Parent->Address : "Root")
			<< '\n';
		if (!Aliases.empty()) oss << str::PresentList(Aliases, "Aliases: ") << '\n';
		if (Usage) oss << "Usage: " << *Usage << '\n';
		if (Desc) oss << "Description: " << *Desc << '\n';
		if (!Examples.empty()) oss << str::PresentList(Examples, "Examples:", "\n    ", "\n    ", "\n");
		if (LongDesc) oss << *LongDesc << '\n';
		if (Children.size() > 0) {
			if (Children.size() == 1) oss << "1 child: [ " << Children[0]->Address << " ]";
			else oss << Children.size() << str::PresentList(GetChildAddresses(), " children: ");
			oss << '\n';
		}*//*
		oss << "\n * [" << Address << "] (" << Group << ")"
			<< (Implemented ? "" : " [WIP]")
			<< ":  *\n\n";*/

		oss << str::ToIndexLine("Address:", Address, col, max, true) << (Implemented ? "" : " [WIP]") << '\n'
			<< str::ToIndexLine("Group:", Group, col, max, true) << '\n'
			<< str::ToIndexLine("Parent:", Parent ? Parent->Address : "Root", col, max, true) << '\n'
			<< str::ToIndexLine("Aliases:", Aliases.empty() ? "(none)" : str::PresentList(Aliases), col, max) << '\n'
			<< str::ToIndexLine("Usage:", *Usage, col, max, true)
			<< "\n\n"
			<< str::ToIndexLine("Description:", Desc.value_or("(none)"), col, max, false);
		if (!Examples.empty()) oss << "\n\n" << str::ToIndexLine("Examples:", str::PresentList(Examples, "", "\n", "", ""), col, max, false);
		if (LongDesc) oss << "\n\n" << *LongDesc;
		if (Children.size() > 0) oss << "\n\n" << str::ToIndexLine("Children:", str::PresentList(GetChildAddresses(), "", "\n", "", ""), col, max, false);

		fmt::TextBox full(oss.str(), Address, fmt::TextAlign::Left, fmt::TextAlign::Left, 1, boxhpad);
		return full.PrintStr(boxw);
		//return oss.str();
	}

	std::string ReplCommand::PrintTree(std::string namePrefix, std::string listPrefix, HelpAttribute help, std::size_t col, std::size_t total) {
		std::ostringstream oss;
		//oss << namePrefix << '[' << Name << "]\n";
		std::string line = namePrefix + "[" + Name + "]";

		switch (help) {
		case HelpAttribute::None: oss << line; break;
		case HelpAttribute::Aliases:			
			oss << str::ToIndexLine(line, (Aliases.size() == 0 ? "-" : str::PresentList(Aliases)), col, total, true);
			break;
		case HelpAttribute::Description:
			oss << str::ToIndexLine(line, Desc.value_or("-"), col, total, true);
			break;
		case HelpAttribute::Examples:
			oss << str::ToIndexLine(line, (Examples.size() == 0 ? "-" : str::PresentList(Examples, "", "', '", "'", "'")), col, total, true);
			break;
		case HelpAttribute::Full:
			oss << line; break;
		case HelpAttribute::Implemented:
			oss << str::ToIndexLine(line, Implemented ? "[x]" : "[ ]", col, total, true);
			break;
		case HelpAttribute::LongDescription:
			oss << str::ToIndexLine(line, LongDesc.value_or("-"), col, total, true);
			break;
		case HelpAttribute::Usage:
			oss << str::ToIndexLine(line, Usage.value_or("-"), col, total, true);
			break;
		default: oss << line;
		}
		oss << '\n';

		for (size_t i = 0; i < Children.size(); i++) {
			if (i == Children.size() - 1) oss << Children[i]->PrintTree(listPrefix + " └─", listPrefix + "   ", help, col, total);
			else oss << Children[i]->PrintTree(listPrefix + " ├─", listPrefix + " │ ", help, col, total);
		}
		return oss.str();
	}
}