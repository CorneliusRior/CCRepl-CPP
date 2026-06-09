#include "pch.h"
#include <iostream>
#include <CCRepl/ReplContext.h>

namespace CCRepl {

	ReplCommand* ReplContext::FindCommand(const std::string& input) {
		ReplCommand* cmd;
		auto it = CommandReg.find(str::ToLower(input));
		if (it != CommandReg.end()) cmd = &it->second;
		else {
			auto alsIt = AliasReg.find(str::ToLower(input));
			if (alsIt != AliasReg.end()) cmd = alsIt->second;
			else throw ReplUserException(std::format("No command found with address or alias '{}', type 'help' to see commands.", input));
		}
		return cmd;
	}

	// Runs a command with token input
	void ReplContext::Execute(const CommandTokens& tokens) {
		try {
			ReplCommand* cmd = FindCommand(tokens.commandHead);
			if (!cmd->CanExecute()) {
				WriteLine(std::format("Command '{}' has no execution function: Cannot execute.", cmd->Address));
				return;
			}
			CommandArgs args(*this, cmd, tokens.args, tokens.opts);
			cmd->Execute(*this, args);
		}
		catch (const ReplUserException& ex) { WriteLine(std::format("User error: {}", ex.what())); }
		catch (const ReplException& ex) { WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (const ReplCancel& ex) { WriteLine(std::format("Cancelled: {}", ex.what())); }
		catch (const std::runtime_error& ex) { WriteLine(std::format("Error: {}", ex.what())); }
		catch (...) { WriteLine("Unknown error."); }
	}

	// Overload which parses raw text into tokens first:
	void ReplContext::Execute(const std::string& input) {
		try { 
			CommandTokens tk = TokenizeParen(input);
			Execute(tk);
		}
		catch (const ReplUserException& ex) { WriteLine(std::format("User error: {}", ex.what())); }
		catch (const ReplException& ex) { WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (const ReplCancel& ex) { WriteLine(std::format("Cancelled: {}", ex.what())); }
		catch (const std::runtime_error& ex) { WriteLine(std::format("Error: {}", ex.what())); }
		catch (...) { WriteLine("Unknown error."); }		
	}

	bool ReplContext::Test(const CommandTokens& tokens, bool run) {
		bool ok;
		try {			
			ReplCommand* cmd = FindCommand(tokens.commandHead);
			if (cmd->CanTest()) {
				CommandArgs args(*this, cmd, tokens.args, tokens.opts);
				ok = cmd->Test(*this, args);
			}
			else {				
				WriteLine(std::format("Command '{}' has no test function: Deemed success.", cmd->Address));
				ok = true;
			}
		}
		catch (const ReplUserException& ex) { WriteLine(std::format("User error: {}", ex.what())); ok = false; }
		catch (const ReplException& ex) { WriteLine(std::format("Repl Error: {}", ex.what())); ok = false; }
		catch (const ReplCancel& ex) { WriteLine(std::format("Cancelled: {}", ex.what())); }
		catch (const std::runtime_error& ex) { WriteLine(std::format("Error: {}", ex.what())); ok = false; }
		catch (...) { WriteLine("Unknown error."); ok = false; }

		if (ok) {
			WriteLine(std::format("[SUCCESS]: '{}'{}.", tokens.commandHead, (run ? ": Running" : "")));
			if (run) Execute(tokens);
		}
		else WriteLine(std::format("[FAILURE]: '{}'{}.", tokens.Print(), (run ? ": Not running" : "")));
		return ok;
	}

	bool ReplContext::Test(const std::string& input, bool run) {
		try {
			CommandTokens tk = TokenizeParen(input);
			return Test(tk, run);
		}
		catch (const ReplUserException& ex) { WriteLine(std::format("User error: {}", ex.what())); }
		catch (const ReplException& ex) { WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (const ReplCancel& ex) { WriteLine(std::format("Cancelled: {}", ex.what())); }
		catch (const std::runtime_error& ex) { WriteLine(std::format("Error: {}", ex.what())); }
		catch (...) { WriteLine("Unknown error."); }
		return false;
	}

	void ReplContext::CloseApp() {
		running = false;
	}

	void ReplContext::Clear(const std::string& text) const {
		if (ReqClear) ReqClear(text);
		else WriteLine("Attempted to clear, but ReqClear not set.");
	}

	std::string ReplContext::ReadLine(const std::string& prompt) {
		if (ReqReadLine) return ReqReadLine(prompt);
		else throw ReplException("ReqReadLine not set.");
	}

	std::string ReplContext::RootTree(HelpAttribute att, std::size_t total) {
		auto GenerateTree = [&](HelpAttribute a, std::size_t col = 50) {
			std::ostringstream oss;
			oss << "[Root]\n";
			for (std::size_t i = 0; i < RootCommands.size(); i++) {
				if (i == RootCommands.size() - 1) oss << RootCommands[i]->PrintTree(" └─", "   ", a, col, total);
				else oss << RootCommands[i]->PrintTree(" ├─", " │ ", a, col, total);
			}
			return oss.str();
			};
		std::string emptyTree = GenerateTree(HelpAttribute::None);
		if (att == HelpAttribute::None) return emptyTree;
		return GenerateTree(att, str::MaxLength(emptyTree) + 5);		
	}

	bool ReplContext::Confirm(const std::string& prompt, const std::string& retryPrompt) {
		bool r;
		while (true) {
			if (parsers::TryBool(ReadLine(prompt), r)) return r;
			WriteLine(retryPrompt);
		}
	}

	void ReplContext::SetCaretVis(bool visible) {
		if (ReqSetCaretVis) ReqSetCaretVis(visible);
	}

	void ReplContext::WriteStatus(std::string text) {
		// Invoke ReqWriteStatus with msg:
		std::size_t newLen = str::StrLength(text);
		if (newLen > lastStatus)  {
			for (std::size_t i = newLen; i < lastStatus; i++) {
				text += ' ';
			}
		}
		lastStatus = newLen;
		
		if (ReqWriteStatus) ReqWriteStatus(text);
		else {
			Write("\r");
			Write(text);
		}
	}

	void ReplContext::ClearStatus(std::string text) {
		for (size_t i = str::StrLength(text); i < lastStatus; i++) text += " ";
		lastStatus = 0;

		if (ReqClearStatus) ReqClearStatus(text);
		else if (ReqWriteStatus) ReqWriteStatus(text + '\n');
		else {
			Write("\r");
			WriteLine(text);
		}
	}

	// Overrides:
	void ReplContext::WriteLine(double value) const { std::cout << value << std::endl; }
	void ReplContext::Write(double value) const { std::cout << value; }

	void ReplContext::RegCmd(ReplCommand cmd) {
		ReplCommand* reg = AssignAddresses(cmd, nullptr);
		AssignAliases(*reg, "");
		RootCommands.push_back(reg->RegPtr);
	}

	void ReplContext::RegCmd(CommandSet* cmdSet) {
		for (ReplCommand& cmd : cmdSet->GetCommands()) RegCmd(std::move(cmd));
	}

	ReplCommand* ReplContext::AssignAddresses(ReplCommand& cmd, ReplCommand* parent) {
		cmd.Parent = parent;
		
		// Generate and assign address:
		//cmd.Address = parentAddress + (parentAddress.empty() ? "" : ".") + cmd.Name;
		cmd.Address = parent ? parent->Address + "." + cmd.Name : cmd.Name;

		// Build usage statement:
		std::ostringstream oss;
		oss << cmd.Address;
		for (std::unique_ptr<IArgSpec>& arg : cmd.ArgSpecs) {
			oss << ' ' << arg->Print();
		}
		for (std::string& opt : cmd.Options) {
			oss << ' ' << opt;
		}
		cmd.Usage = oss.str();

		// Assign group:
		cmd.Group = cmd.Group.empty() ? (parent ? parent->Group : "Ungrouped") : cmd.Group;

		// Register it & define registration pointer:
		auto [it, inserted] = CommandReg.emplace(str::ToLower(cmd.Address), std::move(cmd));
		ReplCommand& reg = it->second;
		reg.RegPtr = &reg;

		// Do the same for each child:
		for (ReplCommand& c : reg.ChildrenInit) reg.Children.push_back(AssignAddresses(c, &reg));
		return &reg;
	}

	void ReplContext::AssignAliases(ReplCommand& cmd, const std::string& parentAddress) {
		std::string prefix = parentAddress + (parentAddress.empty() ? "" : ".");

		// Add canon name:
		AliasReg.emplace(str::ToLower(prefix + cmd.Name), &cmd);
		for (ReplCommand* c : cmd.Children) AssignAliases(*c, prefix + cmd.Name);

		// Add aliases:
		for (const std::string a : cmd.Aliases) {
			AliasReg.emplace(str::ToLower(prefix + a), &cmd);
			for (ReplCommand* c : cmd.Children) AssignAliases(*c, prefix + a);
		}
	}
}