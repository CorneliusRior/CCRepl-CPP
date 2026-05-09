#pragma once

#include "IArgSpec.h"

namespace CCRepl {

	class ReplCommand;
	class ReplContext;

	// Generated whenever a command is called.
	class CommandArgs {
	public:

		std::vector<std::unique_ptr<IArgValue>> Args;
		std::vector<std::string> Options;
		std::string CommandAddress;

		// Constructor: (CommandArgs Args(*this, cmd, tokens, options), something like that)
		CommandArgs(ReplContext& ctx, ReplCommand* cmd, const std::vector<std::string> args, const std::vector<std::string> opt);

		bool HasOption(const std::string& opt);

		template<typename... Ts>
		bool Opt(const Ts&... opts) { return (HasOption(opts) || ...); }

		template<typename T>
		std::optional<T> Get(std::size_t pos) const {
			if (pos >= Args.size()) throw ReplException(std::format("Argument out of range. Args.Size() = {}, pos = {}, in command '{}'.", Args.size(), pos, CommandAddress));
			auto* arg = dynamic_cast<ArgValue<T>*>(Args[pos].get());
			if (!arg) throw ReplException("Argument type mismatch.");
			return arg->Value;
		}
		
		template<typename T>
		T GetOr(std::size_t pos, T fallback) const {
			std::optional<T> value = Get<T>(pos);
			if (!value) return fallback;
			return *value;
		}

		template<typename T>
		T GetRequired(std::size_t pos) const {
			std::optional<T> value = Get<T>(pos);
			if (!value) throw ReplException(std::format("Required value not present. pos = '{}', in command '{}'.", pos, CommandAddress));
			return *value;
		}
	};

}