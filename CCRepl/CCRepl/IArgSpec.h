#pragma once

#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "ReplException.h"
#include "str.h"

namespace CCRepl {
	
	/*
	IArgSpec and components.
	*/

	// Argument Value: What command handlers actually use.
	class IArgValue {
	public:
		virtual ~IArgValue() = default;
	};
	
	template<typename T>
	class ArgValue : public IArgValue {
	public:
		std::optional<T> Value;
		explicit ArgValue(std::optional<T> value) : Value(std::move(value)) {}
	};


	// Argument Mode: What kind of argument it is, required &c.
	enum class ArgMode {
		Required,			// Must be in the command arguments
		RequiredPrompt,		// If not present in args, prompts for it, or default if == cancel strings
		Optional,			// If not present in args, reverts to default
		OptionalPrompt		// If not presnet in args, prompts for it, or default if == cancel strings
	};

	std::string ToString(ArgMode mode);
	bool IsPrompt(ArgMode mode);
	bool IsRequired(ArgMode mode);
	char ArgModeOpen(ArgMode mode);
	char ArgModeClose(ArgMode mode);


	// Prompt Info: Information about how to prompt user for missing data.
	struct PromptInfo {
		std::string prompt;
		std::string retryPrompt;
		std::vector<std::string> cancelStrings = { "\\" };
	};

	// Argument specification: Stored in ReplCommand for verification/formatting.
	class IArgSpec {
	public:
		virtual ~IArgSpec() = default;
		std::string Name = "";
		std::string TypeString = "";
		ArgMode Mode = ArgMode::Required;
		PromptInfo PmtInfo = { "", "", { "\\" } };
		virtual std::unique_ptr<IArgValue> Parse(const std::string& text) const = 0;
		virtual std::unique_ptr<IArgValue> Fallback() const = 0;
		//virtual std::string TypeString() const = 0;
		virtual std::string Print() const = 0;

	protected:
		void GeneratePrompt();
	};

	template<typename T>
	class ArgSpec : public IArgSpec {
	public:
		using Parser = std::function<bool(const std::string&, T&)>;

		// Constructor:
		ArgSpec(std::string name, ArgMode mode, Parser parser, std::optional<T> fallback, PromptInfo pmtInfo, std::optional<std::string> typeString = std::nullopt) :
			parser_(std::move(parser)), fallback_(std::move(fallback)) {
			Name = std::move(name);
			Mode = mode;
			PmtInfo = pmtInfo;
			TypeString = typeString.value_or(str::TypeString<T>());

			if (IsPrompt(Mode)) GeneratePrompt();
		}
	
		std::unique_ptr<IArgValue> Parse(const std::string& text) const override {
			T value{};
			if (parser_(text, value)) return std::make_unique<ArgValue<T>>(value);
			throw ReplUserException(std::format("Cannot parse argument '{}': '{}'", Print(), text));
		}

		std::unique_ptr<IArgValue> Fallback() const override {
			return std::make_unique<ArgValue<T>>(fallback_);
		}

		//std::string TypeString() const override { return str::TypeString<T>(); }

		std::string Print() const override {
			std::ostringstream oss;
			oss << ArgModeOpen(Mode) << TypeString << ' ' << Name << ArgModeClose(Mode);
			return oss.str();
		}
	
	private:
		Parser parser_;
		std::optional<T> fallback_;
	};

}