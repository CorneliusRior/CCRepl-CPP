#pragma once

#include <format>
#include <string>
#include <unordered_set>
#include <vector>
#include "fmt.h"
#include "parsers.h"
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

		std::string Print() const;
		std::string Print_ML() const;
	};	

	struct ScriptStatement {
		CommandTokens tokens;
		std::size_t stmtIndex = 0;
		std::size_t startLine = 0;
		std::size_t endLine = 0;

		std::string Print() const;
		std::string Print_ML() const;
	};

	// List out a vector (not a script, we could turn this into a script later):
	std::string PrintSStmtList(const std::vector<ScriptStatement>& lst);

	/*
	Statement at the start of the script. A statement should look something like this:

	ScriptMetaData (
		Format: parV1,
		Name: CPPTest,
		Author: Cornelius,
		Created: 2026-05-14
	);

	The initial starting bits should be optional, but we'll cater to them. These do need to be in order though.

	*/
	struct ScriptMetaData {
		std::string Format;
		std::string Name;
		std::string Author;
		std::tm Created;

		ScriptMetaData(std::vector<std::string> args);

		std::string Print() const;
	};	

	// For tokenizing commands from CLI:
	CommandTokens TokenizeParen(const std::string& input);			

	// Create a list of tokens to be converted into a script later:
	std::vector<ScriptStatement> TokenizeScript(const std::string& text);
}