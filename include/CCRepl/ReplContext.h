#pragma once
#include <future>
#include <map>
#include <typeindex>
#include <CCRepl/CommandSet.h>
#include <CCRepl/Tokenizers.h>
#include <CCRepl/ReplCommand.h>
#include <CCRepl/Script.h>
#include <util/parsers.h>

#define CTX_WAIT_SPIN(T, function, message, doneMessage) \
	ctx.WaitSpinner<T>([&]() { return function; }, message, doneMessage)

#define CTX_ADD_SVC(Type, ...) \
	ctx.AddService<Type>(std::make_shared<Type>(__VA_ARGS__))

namespace CCRepl {

	class ReplContext {
	public:

		bool running = true;
		std::optional<std::size_t> MaxWidth;
		std::string AboutStr = "No 'About' statement.";
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
		std::string ReadLine(const std::string& prompt = "");
		std::function<void(const std::string&)> ReqClear;
		std::function<void(const std::string&)> ReqWriteLine;
		std::function<void(const std::string&)> ReqWrite;
		std::function<std::string(const std::string&)> ReqReadLine;

		std::string RootTree(HelpAttribute att = HelpAttribute::None, std::size_t total = 180);
		bool Confirm(const std::string& prompt = "(Y/N): ", const std::string& retryPrompt = "Cannot parse, please try again.");

		// Status:
		void SetCaretVis(bool visible);
		void WriteStatus(std::string text);
		void ClearStatus(std::string text = "");
		std::function<void(bool)> ReqSetCaretVis;
		std::function<void(std::string text)> ReqWriteStatus;
		std::function<void(std::string text)> ReqClearStatus;

		template<typename... Txt>
		void WriteLine(Txt&&... text) const {			
			std::ostringstream oss;
			auto WriteOne = [&](auto&& item) {
				if (oss.tellp() > 0) oss << " ";
				oss << std::forward<decltype(item)>(item);
			};
			(WriteOne(std::forward<Txt>(text)), ...);

			if (ReqWriteLine) ReqWriteLine(oss.str());
			else throw ReplException("ReqWriteLine not set.");
		}

		void Write(const std::string& text) const {
			ReqWrite(text);
		}

		template<typename... Txt>
		void Write(Txt&&... text) const {
			std::ostringstream oss;
			auto WriteOne = [&](auto&& item) {
				if (oss.tellp() > 0) oss << " ";
				oss << std::forward<decltype(item)>(item);
				};
			(WriteOne(std::forward<Txt>(text)), ...);

			if (ReqWriteLine) ReqWrite(oss.str());
			else throw ReplException("ReqWriteLine not set.");
		}

		template<typename T>
		ReplContext& operator<<(const T& text) {
			Write(text);
			return *this;
		}

		/*
		Service Storage.

		Add like:
		DataService data;
		ctx.AddService<DataService>(std::make_shared<DataService>(data));

		ctx.AddService<DataService>(std::make_shared<DataService(std::move(data)));
		could also be necessary.

		Or use Macro:
		CTX_ADD_SVC(DataService); (add args needed after, e.g. CTX_ADD_SVC(DataService, path, mode); &c.)

		Get like:
		auto data = ctx.GetService<DataService>();
		*/
		template <typename T>
		void AddService(std::shared_ptr<T> service) { serviceMap_[typeid(T)] = service; }

		template <typename T>
		std::shared_ptr<T> GetService() { return std::static_pointer_cast<T>(serviceMap_.at(typeid(T))); }

		// Request input: For when you want input inside of the command.
		template<typename T>
		T RequireInput(std::function<bool(const std::string&, T&)> parser, const std::string& prompt = "", const std::string& retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" }) {
			while (true) {
				std::string input = ReadLine(prompt);
				if (str::InVector(input, cancelStrings)) throw ReplCancel(std::format("'{}' in cancel strings.", input));
				T value{};
				if (parser(input, value)) return value;
				WriteLine(retryPrompt);
			}
		}

		template<typename T>
		T RequestInput(std::function<bool(const std::string&, T&)> parser, T fallback, const std::string& prompt = "", const std::string& retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" }) {
			try { return RequireInput(parser, prompt, retryPrompt, cancelStrings); }
			catch (const ReplCancel&) { return fallback; }
		}

		template<typename T>
		// Returns std::optional<T>. To get actual value, please use RequireInput<T> or RequestInput<T>
		std::optional<T> RequestInput(std::function<bool(const std::string&, T&)> parser, const std::string& prompt = "", const std::string& retryPrompt = "", std::vector<std::string> cancelStrings = { "\\" }) {
			while (true) {
				std::string input = ReadLine(prompt);
				if (str::InVector(input, cancelStrings)) return std::nullopt;
				T value{};
				if (parser(input, value)) return value;
				WriteLine(retryPrompt);
			}
		}

		// Waiting:
		template<typename T>
		T WaitSpinner(std::future<T> ft, const std::string& message = "Processing", const std::string& doneMessage = "Done.") {
			std::string msg = message.empty() ? "" : message + ' ';
			const std::string frames[] = {
				msg + '|',
				msg + '/',
				msg + '-',
				msg + '\\',
			};
			int i = 0;

			SetCaretVis(false);
			while (ft.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
				WriteStatus(frames[i++ % 4]);

			T r = ft.get();
			ClearStatus(doneMessage);
			SetCaretVis(true);
			return r;
		}

		template<typename T>
		T WaitSpinner(std::function<T()> func, const std::string& message = "Processing", const std::string& doneMessage = "Done.") {
			return WaitSpinner<T>(std::async(std::launch::async, func), message, doneMessage);
		}

	private:

		std::size_t lastStatus = 0;
		void RegCmd(ReplCommand cmd);
		void RegCmd(CommandSet* cmdSet);
		ReplCommand* AssignAddresses(ReplCommand& cmd, ReplCommand* parent);
		void AssignAliases(ReplCommand& cmd, const std::string& parentAddress);

		std::unordered_map<std::type_index, std::shared_ptr<void>> serviceMap_;

	};

}