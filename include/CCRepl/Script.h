#pragma once
#include <CCRepl/CommandArgs.h>
#include <CCRepl/Tokenizers.h>
#include <util/fmt.h>
#include <format>
#include <filesystem>
#include <string>
#include <map>

#define SCP_PRS_UPD(msg) \
	if (!silent) ctx.WriteLine(std::format("({}/{}): {}", stmt.stmtIndex, Statements.size(), msg))

#define SCP_PRS_ERR(msg) \
	{ \
	errs.push_back(std::format("\033[2m({}/{}):\033[0m {}", stmt.stmtIndex, Statements.size(), msg)); \
	if (!silent) ctx.WriteLine(std::format("({}/{}) [FAILURE] ('{}'): {}", stmt.stmtIndex, Statements.size(), stmt.Args.CommandAddress, msg)); \
	} 

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
		std::string PrintFull() const;	// Shows all statements.

		static std::vector<fmt::TextTableColumn> GetTableColumns();
		std::vector<std::string> GetTableRow() const;
	};

	Script TextToScript(ReplContext& ctx, const std::string& text);
	ScriptMetaData ReadMetaData(const std::filesystem::path& path);

	inline const std::unordered_set<char> WhiteSpace = { ' ', '\n', '\t', '\r' };
	
}