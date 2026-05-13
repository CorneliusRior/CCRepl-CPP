#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include "ReplException.h"
#include "str.h"

#define SCRIPT_ERROR(msg) \
	throw ScriptException(std::format("Script Error (Statement #{}, Line #{}, Char {} '{}'): {}", statementIndex, currentLine, i, c, msg))

namespace CCRepl {

	// Struct for result of tokenizer:
	struct CommandTokens {
		std::string commandHead;
		std::vector<std::string> args;
		std::vector<std::string> opts;

		CommandTokens();
		CommandTokens(std::string cmdHead, std::vector<std::string> a, std::vector<std::string> o);

		std::string Print() const;
	};	

	struct ScriptStatement {
		CommandTokens tokens;
		std::size_t stmtIndex;
		std::size_t startLine;
		std::size_t endLine;

		ScriptStatement();
		ScriptStatement(std::string commandHead, std::vector<std::string> args, std::vector<std::string> opts, std::size_t index, std::size_t start, std::size_t end);
	};

	struct ScriptMetaData {
		std::string Format;
		std::string Name;
		std::string Author;
		std::string Created;
	};

	// For tokenizing commands from CLI:
	CommandTokens TokenizeParen(const std::string& input);			

	// Create a list of tokens to be converted into a script later:
	std::vector<ScriptStatement> TokenizeScript(const std::string& text);
}