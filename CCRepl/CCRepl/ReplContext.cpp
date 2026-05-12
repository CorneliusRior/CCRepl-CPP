#include "pch.h"
#include <iostream>
#include "ReplContext.h"

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
		catch (ReplUserException ex) { WriteLine(std::format("User error: {}", ex.what())); }
		catch (ReplException ex) { WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (std::runtime_error ex) { WriteLine(std::format("Error: {}", ex.what())); }
		catch (...) { WriteLine("Unknown error."); }
	}

	// Overload which parses raw text into tokens first:
	void ReplContext::Execute(const std::string& input) {
		try { 
			CommandTokens tk = TokenizeParen(input);
			Execute(tk);
		}
		catch (ReplUserException ex) { WriteLine(std::format("User error: {}", ex.what())); }
		catch (ReplException ex) { WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (std::runtime_error ex) { WriteLine(std::format("Error: {}", ex.what())); }
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
		catch (ReplUserException ex) { WriteLine(std::format("User error: {}", ex.what())); ok = false; }
		catch (ReplException ex) { WriteLine(std::format("Repl Error: {}", ex.what())); ok = false; }
		catch (std::runtime_error ex) { WriteLine(std::format("Error: {}", ex.what())); ok = false; }
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
		catch (ReplUserException ex) { WriteLine(std::format("User error: {}", ex.what())); }
		catch (ReplException ex) { WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (std::runtime_error ex) { WriteLine(std::format("Error: {}", ex.what())); }
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

	void ReplContext::WriteLine(const std::string& text) const {
		if (ReqWriteLine) ReqWriteLine(text);
		else throw ReplException("ReqWriteLine not set.");
	}

	void ReplContext::Write(const std::string& text) const {
		if (ReqWrite) ReqWrite(text);
		else throw ReplException("ReqWrite not set.");
	}
	
	std::string ReplContext::ReadLine(const std::string& prompt) {
		if (ReqReadLine) return ReqReadLine(prompt);
		else throw ReplException("ReqReadLine not set.");
	}

	std::string ReplContext::RootTree() {
		std::ostringstream oss;
		oss << "[Root]\n";
		for (std::size_t i = 0; i < RootCommands.size(); i++) {
			if (i == RootCommands.size() - 1) oss << RootCommands[i]->PrintTree(" └─", "   ");
			else oss << RootCommands[i]->PrintTree(" ├─", " │ ");
		}
		return oss.str();
	}

	bool ReplContext::Confirm(const std::string& prompt, const std::string& retryPrompt) {
		bool r;
		while (true) {
			if (parsers::TryBool(ReadLine(prompt), r)) return r;
			WriteLine(retryPrompt);
		}
	}

	void ReplContext::HideCaret() {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(h, &info);
		info.bVisible = false;
		SetConsoleCursorInfo(h, &info);
	}

	void ReplContext::ShowCaret() {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(h, &info);
		info.bVisible = true;
		SetConsoleCursorInfo(h, &info);
	}

	void ReplContext::WriteStatus(std::string text) {
		size_t newLen = str::StrLength(text);
		int diff = lastStatus - newLen;
		for (diff; diff > 0; diff--) text += " ";
		std::cout << "\r" << text << std::flush;
		lastStatus = newLen;
	}

	void ReplContext::ClearStatus(std::string text) {
		for (size_t i = str::StrLength(text); i < lastStatus; i++) text += " ";
		std::cout << "\r" << text << std::endl << std::flush;
		lastStatus = 0;
	}

	// Overrides:
	void ReplContext::WriteLine(double value) const { std::cout << value << std::endl; }
	void ReplContext::Write(double value) const { std::cout << value; }

	void ReplContext::RegCmd(ReplCommand cmd) {
		ReplCommand* reg = AssignAddresses(cmd, "");
		AssignAliases(*reg, "");
		RootCommands.push_back(reg->RegPtr);
	}

	void ReplContext::RegCmd(CommandSet* cmdSet) {
		for (ReplCommand& cmd : cmdSet->GetCommands()) RegCmd(std::move(cmd));
	}

	ReplCommand* ReplContext::AssignAddresses(ReplCommand& cmd, const std::string& parentAddress) {
		// Generate and assign address and usage:
		cmd.Address = parentAddress + (parentAddress.empty() ? "" : ".") + cmd.Name;
		std::ostringstream oss;
		oss << cmd.Address;
		for (std::unique_ptr<IArgSpec>& arg : cmd.ArgSpecs) {
			oss << ' ' << arg->Print();
		}
		for (std::string& opt : cmd.Options) {
			oss << ' ' << opt;
		}
		cmd.Usage = oss.str();

		// Register it & define registration pointer:
		auto [it, inserted] = CommandReg.emplace(str::ToLower(cmd.Address), std::move(cmd));
		ReplCommand& reg = it->second;
		reg.RegPtr = &reg;

		// Do the same for each child:
		for (ReplCommand& c : reg.ChildrenInit) reg.Children.push_back(AssignAddresses(c, reg.Address));
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