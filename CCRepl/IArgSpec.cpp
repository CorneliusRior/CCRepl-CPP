#include "pch.h"
#include <CCRepl/IArgSpec.h>

namespace CCRepl {

	std::string ToString(ArgMode mode) {
		switch (mode) {
		case ArgMode::Required: return "Required";
		case ArgMode::RequiredPrompt: return "RequiredPrompt";
		case ArgMode::Optional: return "Optional";
		case ArgMode::OptionalPrompt: return "OptionalPrompt";
		default: return "Unknown";
		}
	}

	bool IsPrompt(ArgMode mode) {
		switch (mode) {
		case ArgMode::Required: return false;
		case ArgMode::RequiredPrompt: return true;
		case ArgMode::Optional: return false;
		case ArgMode::OptionalPrompt: return true;
		default: return false;
		}
	}

	bool IsRequired(ArgMode mode) {
		switch (mode) {
		case ArgMode::Required: return true;
		case ArgMode::RequiredPrompt: return true;
		case ArgMode::Optional: return false;
		case ArgMode::OptionalPrompt: return false;
		default: return false;
		}
	}

	char ArgModeOpen(ArgMode mode) {
		switch (mode) {
		case ArgMode::Required: return '<';
		case ArgMode::RequiredPrompt: return '<';
		case ArgMode::Optional: return '[';
		case ArgMode::OptionalPrompt: return '[';
		default: return '(';
		}
	}

	char ArgModeClose(ArgMode mode) {
		switch (mode) {
		case ArgMode::Required: return '>';
		case ArgMode::RequiredPrompt: return '>';
		case ArgMode::Optional: return ']';
		case ArgMode::OptionalPrompt: return ']';
		default: return ')';
		}
	}

	void IArgSpec::GeneratePrompt() {
		if (PmtInfo.prompt.empty()) {
			std::ostringstream oss;
			oss << "Enter value for " 
				<< Name 
				<< (IsRequired(Mode) ? " (required, " : " (optional, ") 
				<< TypeString << "): ";
			PmtInfo.prompt = oss.str();
		}
		if (PmtInfo.retryPrompt.empty()) {
			std::ostringstream oss;
			oss << "Could not parse, please try again";
			if (PmtInfo.cancelStrings.empty()) oss << ".";
			else oss 
				<< (IsRequired(Mode) ? " (to cancel" : " (for default")
				<< str::PresentList(PmtInfo.cancelStrings, ", type one of the following: ", "', '", "{ '", "' }).");
			PmtInfo.retryPrompt = oss.str();
		}
	}

}