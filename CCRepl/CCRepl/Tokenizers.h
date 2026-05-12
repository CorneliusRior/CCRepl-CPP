#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include "ReplException.h"
#include "str.h"

namespace CCRepl {

	// Struct for result of tokenizer:
	struct CommandTokens
	{
		std::string commandHead;
		std::vector<std::string> args;
		std::vector<std::string> opts;

		std::string Print() const;
	};

	CommandTokens TokenizeParen(const std::string& input);

}