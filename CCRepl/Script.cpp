#include "pch.h"
#include <CCRepl/ReplContext.h>
#include <CCRepl/Script.h>
#include <CCRepl/ScriptTools.h>
#include <util/fmt.h>
#include <util/ansi.h>

namespace CCRepl {

	std::string ScriptToken::Print() const {
		return std::format("Statement #{} (Lines {}-{}): {};", stmtIndex, startLine, endLine, tokens.Print());
	}

	std::string ScriptToken::Print_ML() const {
		return std::format("Statement #{} (Lines {}-{}):\n{};", stmtIndex, startLine, endLine, tokens.Print_ML());
	}

	std::string PrintSStmtList(const std::vector<ScriptToken>& lst) {
		std::ostringstream oss;
		for (const ScriptToken& stmt : lst) {
			oss << stmt.Print_ML() << "\n\n";
		}
		return oss.str();
	}

	ScriptMetaData::ScriptMetaData(std::vector<std::string> args) {
		if (args.size() != 4) throw ScriptException(std::format("Script statement has '{}' items, expected 4.\n{}", args.size(), STR_PRINT_V(args)));

		const static std::vector<char> colons = { ' ', ':', '=', '-' };
		std::vector<std::string> errs;

		auto RemoveLabel = [&](std::string text, const std::string& label) {
			if (str::StartsWith(text, label)) text = str::DropFirstUtf8(text, str::StrLength(label));
			text = str::Trim(text, colons);
			if (text.empty()) errs.push_back(std::format("ScriptMetaData item '{}' is empty.", label));
			return text;
			};

		// We'll just do it manually I guess.
		std::string fmt = RemoveLabel(args[0], "format");
		std::string nme = RemoveLabel(args[1], "name");
		std::string ath = RemoveLabel(args[2], "author");
		std::string dte = RemoveLabel(args[3], "created");

		std::tm crt;
		if (!parsers::TryTime(dte, crt)) errs.push_back(std::format("Could not parse ScriptMetaData item 'created': '{}'", dte));

		if (errs.size() > 0) throw ScriptException(str::PresentList(errs, "Could not parse ScriptMetaData, errors:", "\n", "\n", ""));
		Format = fmt;
		Name = nme;
		Author = ath;
		Created = crt;
	}

	std::string ScriptMetaData::Print() const {
		std::ostringstream oss;
		oss << "\033[36;4mFormat:\033[0m " << Format << '\n';
		oss << "\033[36;4mScript Name:\033[0m " << Name << '\n';
		oss << "\033[36;4mAuthor:\033[0m " << Author << '\n';
		oss << "\033[36;4mCreated:\033[0m " << str::ToDateString(Created) << '\n';
		return oss.str();
	}	

	Script::Script(ReplContext& ctx, std::vector<ScriptToken> tokens) : MetaData(tokens[0].tokens.args) {
		std::vector<std::string> errs;
		for (std::size_t i = 1; i < tokens.size(); i++) {
			try {
				Statements.push_back(ScriptStatement{
						CommandArgs(ctx, tokens[i]),
						tokens[i].stmtIndex,
						tokens[i].startLine,
						tokens[i].endLine
					});
			}
			catch (const ReplUserException& ex) {
				std::string errmsg = std::format("\033[2m({}/{})\033[0;33m '{}'\033[0;31m User error:\033[0m {}\n\033[2m{}\n\033[0m", tokens[i].stmtIndex, tokens.size() - 1, tokens[i].tokens.commandHead, ex.what(), tokens[i].Print_ML());
				errs.push_back(errmsg);
				ctx.WriteLine(errmsg);
			}
		}
		if (!errs.empty()) throw ReplUserException(std::format("Could not parse, {} errors:\n{}", errs.size(), str::PresentList(errs, "", "\n", "")));
	}

	bool Script::Test(ReplContext& ctx, bool silent) {
		std::vector<std::string> errs;
		ctx.WriteLine(Print("Testing Script"));
		for (ScriptStatement& stmt : Statements) {
			if (stmt.Args.Cmd->CanTest()) {
				try {
					if (stmt.Args.Cmd->Test(ctx, stmt.Args)) { SCP_PRS_UPD(std::format("[SUCCESS] ('{}').", stmt.Args.CommandAddress)); }
					else SCP_PRS_ERR("Test function returned false.")
				}
				catch (const ReplUserException& ex) { SCP_PRS_ERR(std::format("\033[31mUser error:\033[1m {}", ex.what())) }
				catch (const ReplException& ex) { SCP_PRS_ERR(std::format("\033[31mRepl error:\033[1m {}", ex.what())) }
				catch (const std::runtime_error& ex) { SCP_PRS_ERR(std::format("\033[31mError:\033[1m {}", ex.what())) }
				catch (...) { SCP_PRS_ERR(std::format("Unknown Error.")) }
			}
			else SCP_PRS_UPD(std::format("Method '{}' has no test function: Deemed Success.", stmt.Args.CommandAddress));			
		}
		if (errs.empty()) {
			ctx.WriteLine("Testing complete: No issues found.");
			return true;
		}
		else {
			ctx.WriteLine(std::format("\nTesting complete: {} issues found:", errs.size()));
			for (std::string err : errs) {
				ctx.WriteLine(err);
			}
			return false;
		}		
	}

	void Script::Execute(ReplContext& ctx) {
		// We presume it's already tested, or forced.
		for (ScriptStatement& stmt : Statements) {
			try {
				ReplCommand* cmd = stmt.Args.Cmd;
				if (!stmt.Args.Cmd->CanExecute()) {
					ctx.WriteLine(std::format("Command '{}' has no execution function: Cannot execute.", cmd->Address));
					continue;
				}
				cmd->Execute(ctx, stmt.Args);
			}
			catch (const ReplUserException& ex) { ctx.WriteLine(std::format("User error at statement {}: {}", stmt.stmtIndex, ex.what())); return ; }
			catch (const ReplException& ex) { ctx.WriteLine(std::format("Repl Error at statement {}: {}", stmt.stmtIndex, ex.what())); return ; }
			catch (const std::runtime_error& ex) { ctx.WriteLine(std::format("Error at statement {}: {}", stmt.stmtIndex, ex.what())); return ; }
			catch (...) { ctx.WriteLine("Unknown error."); return ; }
		}
	}

	std::string Script::Print(const std::string& title) const {
		std::ostringstream oss;
		oss << '\n' << MetaData.Print();
		return fmt::TxtBoxLeft(oss.str(), title);
	}

	std::string Script::PrintFull() const {
		std::ostringstream oss;
		oss << "\033[1;4m" << MetaData.Name << ":\n\n" << ansi::reset
			<< "\033[2mMetaData:\033[0m\n"
			<< MetaData.Print() << "\n"
			<< "\033[2mStatements:\033[0m\n";
		for (const ScriptStatement& stmt : Statements) {
			oss << stmt.stmtIndex << " \033[2m(line " << stmt.startLine << "): \033[0;33m"
				<< stmt.Args.CommandAddress << ansi::reset << " (" << stmt.Args.Args.size() << " arguments);\n";
		}
		oss << "\n\033[2m-- end of script --\033[0m";
		return oss.str();
	}
	
	std::vector<fmt::TextTableColumn> Script::GetTableColumns() {
		return {
			fmt::TextTableColumn("Name:", 25),
			fmt::TextTableColumn("Stmts #:", 8, fmt::TextAlign::Left, fmt::TextAlign::Right),
			fmt::TextTableColumn("Format:", 8),
			fmt::TextTableColumn("Author:", 25),
			fmt::TextTableColumn("Created:", 12)
		};
	}

	std::vector<std::string> Script::GetTableRow() const {
		return {
			MetaData.Name, std::to_string(Statements.size()), MetaData.Format, MetaData.Author, str::ToDateString(MetaData.Created)
		};
	}

	Script TextToScript(ReplContext& ctx, const std::string& text) {
		std::vector<ScriptToken> tokens = ScriptTools::TokenizeScript(text);
		return Script(ctx, tokens);
	}	

	ScriptMetaData ReadMetaData(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file) throw std::runtime_error("Could not open file: " + path.string());
		std::string raw;
		std::getline(file, raw, ';');
		raw += ";";
		std::size_t i = 0;
		CommandTokens tokens = ScriptTools::TokenizeCmd(raw, i);
		return ScriptMetaData(tokens.args);
	}
}
