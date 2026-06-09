#pragma once

#include <format>
#include <string>
#include <unordered_set>
#include <vector>
#include "fmt.h"
#include "parsers.h"
#include "ReplException.h"
#include "str.h"


namespace CCRepl {

	// Struct for result of tokenizer:
	struct CommandTokens {
		std::string commandHead;
		std::vector<std::string> args;
		std::vector<std::string> opts;

		std::string Print() const;
		std::string Print_ML() const;
	};		

	// For tokenizing commands from CLI:
	CommandTokens TokenizeParen(const std::string& input);			


}