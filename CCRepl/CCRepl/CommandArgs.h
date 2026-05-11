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
		
		/// <summary>
		/// Returns true if given string exists in the options vector. Not case sensitive.
		/// </summary>
		/// <param name="opt">Option you are looking for.</param>
		/// <returns></returns>
		bool HasOption(const std::string& opt);

		/// <summary>
		/// Returns true if options vector has a string starting with given string.
		/// </summary>
		/// <param name="opt">Start of option you are looking for.</param>
		/// <returns></returns>
		bool HasOptStart(const std::string& opt);

		/// <summary>
		/// Given a list of strings, returns true if one of these strings exist in the options vector. Useful for aliases.
		/// </summary>
		/// <param name="...opts">List of options you are looking for.</param>
		/// <returns></returns>
		template<typename... Ts>
		bool Opt(const Ts&... opts) { return (HasOption(opts) || ...); }

		/// <summary>
		/// Returns true if options vector has a string starting with one of a given list of strings. Useful for aliases.
		/// </summary>
		/// <param name="...opts">List of starts of options you are looking for.</param>
		/// <returns></returns>
		template<typename... Ts>
		bool OptStrt(const Ts&... opts) { return (HasOptStart(opts) || ...); }

		/// <summary>
		/// Given a list of strings, returns the position of the first of those string to appear in the options vector. If none appear, returns -1.
		/// </summary>
		/// <param name="...opts">Options you are looking for</param>
		/// <returns></returns>
		template<typename... Ts>
		int FirstOptionOf(const Ts&... opts) {
			std::vector<std::string> optsVec = { opts... };
			for (const std::string& o : Options) {
				auto it = std::find_if(optsVec.begin(), optsVec.end(),
					[&o](const std::string& s) { return str::Equals(o, s, false); });
				if (it != optsVec.end()) return static_cast<int>(std::distance(optsVec.begin(), it));
			}
			return -1;
		}

		/// <summary>
		/// Given a list of strings, return the position of the first of those strings to have a option in the options vector start with that string. If none appear, returns -1.
		/// </summary>
		/// <param name="...opts">Start of options you are looking for</param>
		/// <returns></returns>
		template<typename... Ts>
		int FirstOptionStart(const Ts&... opts) {
			std::vector<std::string> optsVec = { opts... };
			for (const std::string& o : Options) {
				auto it = std::find_if(optsVec.begin(), optsVec.end(),
					[&o](const std::string& s) { return str::StartsWith(o, s, false); });
				if (it != optsVec.end()) return static_cast<int>(std::distance(optsVec.begin(), it));
			}
			return -1;
		}

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