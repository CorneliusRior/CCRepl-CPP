#pragma once
#include <format>
#include <string>
#include <map>
#include <CCRepl/CommandArgs.h>
#include <CCRepl/Tokenizers.h>
#include <util/fmt.h>

#define SCRIPT_ERROR(msg) \
	throw ScriptException(std::format("Script Error (Line #{}, Char {} '{}'): {}", FindLine(text, i), i, text[i], msg))

#define SCP_PRS_UPD(msg) \
	if (!silent) ctx.WriteLine(std::format("({}/{}): {}", stmt.stmtIndex, Statements.size(), msg))

#define SCP_PRS_ERR(msg) \
	{ \
	errs.push_back(std::format("\033[2m({}/{}):\033[0m {}", stmt.stmtIndex, Statements.size(), msg)); \
	if (!silent) ctx.WriteLine(std::format("({}/{}) [FAILURE] ('{}'): {}", stmt.stmtIndex, Statements.size(), stmt.Args.CommandAddress, msg)); \
	} 

#define SCP_DB(msg, dbmsg) \
	STR_P(STR_VAR(i), STR_VAR(c), msg); DB(dbmsg)


/*
For script stuff, first thing we will do it try to migrate all of the script tokenization stuff over to here.
*/

namespace CCRepl {

	class ReplContext;
	class CommandArgs;

	struct ScriptToken {
		CommandTokens tokens;
		std::size_t stmtIndex = 0;
		std::size_t startLine = 0;
		std::size_t endLine = 0;

		std::string Print() const;
		std::string Print_ML() const;
	};

	// List out a vector (not a script, we could turn this into a script later):
	std::string PrintSStmtList(const std::vector<ScriptToken>& lst);

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

	struct ScriptStatement {
		CommandArgs Args;
		std::size_t stmtIndex = 0;
		std::size_t startLine = 0;
		std::size_t endLine = 0;
	};

	struct Script {
		ScriptMetaData MetaData;
		std::vector<ScriptStatement> Statements;

		Script(ReplContext& ctx, std::vector<ScriptToken> tokens);

		bool Test(ReplContext& ctx, bool silent = false);
		void Execute(ReplContext& ctx);
		std::string Print(const std::string& title = "Script Info") const;

		static std::vector<fmt::TextTableColumn> GetTableColumns();
		std::vector<std::string> GetTableRow() const;
	};

	Script TextToScript(ReplContext& ctx, const std::string& text);

	//static std::unordered_set<char> Dots = { '.', ' ', ',', '\n', '\t', '\r' };
	inline const std::unordered_set<char> WhiteSpace = { ' ', '\n', '\t', '\r' };

	// Create a list of tokens to be converted into a script later:
	std::vector<ScriptToken> TokenizeScript(const std::string& text);

	// Some helpers for script:
	/* Iterates through until it finds a character, returns everything in between. 
	Returns string if it hits the end of the text, does not consume. Includes text[i].
	E.g.:
	text = "ABC [DEF] GHI";
	i = 4;
	StringUntil(text, i, ']', true) = "[DEF]";
	i == 8;
	*/
	static std::string StringUntil(const std::string& text, std::size_t& i, char c, bool includeLast);

	static std::string StringUntil(const std::string& text, std::size_t& i, std::vector<char> cs, bool includeLast);

	/* Iterates through until it finds a character. Returns true if it finds the character, false if it reaches end of text.
	'i' will be set on that char, does not consume. e.g.: 
	text = "ABC # DEF # GHI";
	i = 4;
	IgnoreUntil(text, i, '#');
	i == 10;	
	*/
	static bool IgnoreUntil(const std::string& text, std::size_t& i, char c);

	/* Iterates through until it finds a character in the cs vector. Returns that char if it finds that char, throws if it reaches end of text.
	'i' will be set to that char, does not consume. e.g.:
	text = "ABC { DEF } GHI";
	i = 4;
	IgnoreUntil(text, i, { '}', '\\' });
	i == 10;	
	*/
	static char IgnoreUntil(const std::string& text, std::size_t& i, std::vector<char> cs);

	// Maybe this should be in tokenizers instead?:
	/* For argument parsing section of script parsing. Everything inside of (...) */
	static std::vector<std::string> TokenizeArgs(const std::string& text, std::size_t& i);

	/* Does the whole command, commandhead, args, and options. */
	static CommandTokens TokenizeCmd(const std::string& text, std::size_t& i);

	// Finds what line of the script you're on.
	static std::size_t FindLine(const std::string& text, const std::size_t& pos);

	// Combinations w/catesian product (for repeat)
	static std::vector<std::map<std::string, std::string>> CartesianProduct(const std::vector<std::string>& varNames, const std::map<std::string, std::vector<std::string>>& repeatVars);
}