#pragma once
#include <map>
#include <typeindex>
#include <Windows.h>
#include "CommandSet.h"
#include "Tokenizers.h"
#include "parsers.h"
#include "ReplCommand.h"

namespace CCRepl {

	class ReplContext {
	public:

		bool running = true;
		std::map<std::string, ReplCommand> CommandReg;
		std::map<std::string, ReplCommand*> AliasReg;
		std::vector<ReplCommand*> RootCommands;
		ReplCommand* CurrentCommand = nullptr;

		// Constructor:
		template<typename... Cmds>
		ReplContext(Cmds&&... cmds) {
			(RegCmd(std::forward<Cmds>(cmds)), ...);
		}

		// IO:
		ReplCommand* FindCommand(const std::string& input);
		void Execute(const CommandTokens& tokens);
		void Execute(const std::string& input);
		bool Test(const CommandTokens& tokens, bool run = false);
		bool Test(const std::string& input, bool run = false);
		void CloseApp();
		void Clear(const std::string& text = "") const;
		void WriteLine(const std::string& text = "") const;
		void Write(const std::string& text = "") const;
		std::string ReadLine(const std::string& prompt = "");
		std::function<void(const std::string&)> ReqClear;
		std::function<void(const std::string&)> ReqWriteLine;
		std::function<void(const std::string&)> ReqWrite;
		std::function<std::string(const std::string&)> ReqReadLine;

		std::string RootTree();
		bool Confirm(const std::string& prompt = "(Y/N): ", const std::string& retryPrompt = "Cannot parse, please try again.");

		// Status:
		void HideCaret();
		void ShowCaret();
		void WriteStatus(std::string text);
		void ClearStatus(std::string text = "");

		// Overloads:
		void WriteLine(double value) const;
		void Write(double value) const;

		/*
		Service Storage.

		Add like:
		DataService data;
		ctx.AddService<DataService>(std::make_shared<DataService>(data));

		Get like:
		auto data = ctx.GetService<DataService>();
		*/
		template <typename T>
		void AddService(std::shared_ptr<T> service) { serviceMap_[typeid(T)] = service; }

		template <typename T>
		std::shared_ptr<T> GetService() { return std::static_pointer_cast<T>(serviceMap_.at(typeid(T))); }

	private:

		std::size_t lastStatus = 0;
		void RegCmd(ReplCommand cmd);
		void RegCmd(CommandSet* cmdSet);
		ReplCommand* AssignAddresses(ReplCommand& cmd, const std::string& parentAddress);
		void AssignAliases(ReplCommand& cmd, const std::string& parentAddress);

		std::unordered_map<std::type_index, std::shared_ptr<void>> serviceMap_;

	};

}