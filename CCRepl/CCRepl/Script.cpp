#include "pch.h"
#include "ReplContext.h"
#include "script.h"

namespace CCRepl {

	std::string ScriptToken::Print() const {
		return std::format("Statement #{} (Lines {}-{}): {};", stmtIndex, startLine, endLine, tokens.Print());
	}

	std::string ScriptToken::Print_ML() const {
		return std::format("Statement #{} (Lines {}-{}):\n{};", stmtIndex, startLine, endLine, tokens.Print_ML());
	}

	std::string PrintSStmtList(const std::vector<ScriptToken>& lst) {
		std::ostringstream oss;
		for (ScriptToken stmt : lst) {
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
		oss << "Format: " << Format << '\n';
		oss << "Script Name: " << Name << '\n';
		oss << "Author: " << Author << '\n';
		oss << "Created: " << str::ToDateString(Created) << '\n';
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
			catch (ReplUserException ex) {
				std::string errmsg = std::format("({}/{}) '{}' User error: {}\n{}\n", tokens[i].stmtIndex, tokens.size() - 1, tokens[i].tokens.commandHead, ex.what(), tokens[i].Print_ML());
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
				catch (ReplUserException ex) { SCP_PRS_ERR(std::format("User error: {}", ex.what())) }
				catch (ReplException ex) { SCP_PRS_ERR(std::format("Repl error: {}", ex.what())) }
				catch (std::runtime_error ex) { SCP_PRS_ERR(std::format("Error: {}", ex.what())) }
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
		try {
			for (ScriptStatement& stmt : Statements) {
				ReplCommand* cmd = stmt.Args.Cmd;
				if (!stmt.Args.Cmd->CanExecute()) {
					ctx.WriteLine(std::format("Command '{}' has no execution function: Cannot execute.", cmd->Address));
					continue;
				}
				cmd->Execute(ctx, stmt.Args);
			}
		}
		catch (ReplUserException ex) { ctx.WriteLine(std::format("User error: {}", ex.what())); }
		catch (ReplException ex) { ctx.WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (std::runtime_error ex) { ctx.WriteLine(std::format("Error: {}", ex.what())); }
		catch (...) { ctx.WriteLine("Unknown error."); }
	}

	std::string Script::Print(const std::string& title) const {
		std::ostringstream oss;
		oss << '\n' << MetaData.Print();
		return fmt::TxtBoxLeft(oss.str(), title);
	}
	
	std::vector<fmt::TextTableColumn> Script::GetTableColumns() {
		return {
			fmt::TextTableColumn("Name:", 30),
			fmt::TextTableColumn("Stmts #:", 10, fmt::TextAlign::Left, fmt::TextAlign::Right),
			fmt::TextTableColumn("Format:", 8),
			fmt::TextTableColumn("Author:", 30),
			fmt::TextTableColumn("Created:", 10)
		};
	}

	std::vector<std::string> Script::GetTableRow() const {
		return {
			MetaData.Name, std::to_string(Statements.size()), MetaData.Format, MetaData.Author, str::ToString(MetaData.Created)
		};
	}

	Script TextToScript(ReplContext& ctx, const std::string& text) {
		std::vector<ScriptToken> tokens = TokenizeScript(text);
		// delete:
		std::this_thread::sleep_for(std::chrono::seconds(3));
		return Script(ctx, tokens);
	}

	std::vector<ScriptToken> TokenizeScript(const std::string& text) {
		/*
		Rules:
		Comments can be formed as either block comments starting & ending with '#', or a line starting with '//', but only outside of statements.
		Statements start at the first letter of the next one, end end with ';'.
		New paragraphs or '\n' are allowed everywhere, unless we have some reason to believe that it's in a quote or brackets or something, we ignore it.
		Otherwise, we follow the general rules of TokenizaParen.
		Like TokenizeParen, we're not doing UTF-8 stuff, just normal input.
		*/

		std::vector<ScriptToken> r;
		std::size_t scriptLength = text.size();
		if (scriptLength == 0) return r;

		std::size_t statementIndex = 0;
		std::size_t currentLine = 1;
		CommandTokens currentToken;
		ScriptToken currentStatement;

		std::ostringstream cmdss;
		std::ostringstream argss;
		std::ostringstream optss;

		std::unordered_set<char> dots = { '.', ' ', ',', '/', '\n', '\t', '\r' };
		std::unordered_set<char> whiteSpace = { ' ', '\n', '\t', '\r' };

		auto NextChar = [&scriptLength, &text](std::size_t i, char c, std::size_t steps = 1) {
			return i + steps < scriptLength && text[i + steps] == c;
			};
		auto AddArg = [&currentToken, &argss]() {
			currentToken.args.push_back(argss.str());
			argss.str("");
			argss.clear();
			};
		auto AddOpt = [&currentToken, &optss]() {
			currentToken.opts.push_back(optss.str());
			optss.str("");
			optss.clear();
			};

		enum class State {
			CmtLine,		// Comment
			CmtBlock,
			InterStmt,		// Between statements (comments & cmd allowed)
			Cmd,			// CmdHead
			CmdInter,
			ArgInter,
			Free,			// Comma seperated
			Quote,			// Quote seperated
			Brace,			// Brace seperated { } (more formatting)
			AwaitComma,		// After quote or brace, throws if next not space, comma, \n
			Opt,			// Reading an option
			OptInter		// Between an option.
		};
		State st = State::InterStmt;

		auto AddStmt = [&]() {
			currentToken.commandHead = str::Trim(cmdss.str(), '.');
			cmdss.str("");
			cmdss.clear();

			if (!argss.str().empty()) AddArg();
			if (!optss.str().empty()) AddOpt();

			currentStatement.tokens = currentToken;
			currentStatement.endLine = currentLine;
			r.push_back(currentStatement);

			CommandTokens newTk{};
			ScriptToken newSt{};
			currentToken = newTk;
			currentStatement = newSt;

			st = State::InterStmt;
			};

		for (std::size_t i = 0; i < scriptLength; i++) {
			char c = text[i];
			if (c == '\n') currentLine++;

			switch (st) {

			case State::CmtLine:
				if (c == '\n') st = State::InterStmt;
				continue;

			case State::CmtBlock:
				if (c == '#') st = State::InterStmt;
				continue;

			case State::InterStmt:
				if (whiteSpace.contains(c)) continue;
				switch (c) {
				case '/':
					if (NextChar(i, '/')) st = State::CmtLine;
					else SCRIPT_ERROR("Expected '/' (\"//\" denotes line comment).");
					continue;
				case '#':
					st = State::CmtBlock;
					continue;
				case ';':
					SCRIPT_ERROR("';': Expected statement.");
				default:
					currentStatement.startLine = currentLine;
					currentStatement.stmtIndex = statementIndex++;
					cmdss << c;
					st = State::Cmd;
					continue;
				}

			case State::Cmd:
				if (dots.contains(c)) {
					cmdss << '.';
					st = State::CmdInter;
					continue;
				}
				switch (c) {
				case '(':
					currentToken.commandHead = cmdss.str();
					st = State::ArgInter;
					continue;
				case ';':
					AddStmt();
					continue;
				default:
					cmdss << c;
					continue;
				}

			case State::CmdInter:
				if (dots.contains(c)) continue;
				switch (c) {
				case '(':
					currentToken.commandHead = cmdss.str();
					st = State::ArgInter;
					continue;
				case ';':
					AddStmt();
					continue;
				default:
					cmdss << c;
					st = State::Cmd;
					continue;
				}

			case State::ArgInter:
				if (whiteSpace.contains(c)) continue;
				switch (c) {
				case ',': AddArg(); continue; // Blank argument;
				case ')': st = State::OptInter;	continue;	// End of arguments (no new)
				case '"': st = State::Quote;	continue;
				case '{': st = State::Brace;	continue;
				default:
					st = State::Free;
					argss << c;
					continue;
				}
				continue;

			case State::Free:
				switch (c) {
				case ',':
					AddArg();
					st = State::ArgInter;
					continue;
				case ')':
					// This statement in case it is the last argument in a statement and we end it with \n instead of ','
					if (text[i - 1] == '\n') {
						currentToken.args.push_back(str::DropLastUtf8(argss.str(), 1));
						argss.str("");
						argss.clear();
					}
					else AddArg();
					st = State::OptInter;
					continue;
				default:
					argss << c;
					continue;
				}

			case State::Quote:
				switch (c) {
				case '"':
					AddArg();
					st = State::AwaitComma;
					continue;
				case '\\':
					argss << (i + 1 < scriptLength ? text[++i] : '\\');
					continue;
				default:
					argss << c;
					continue;
				}

			case State::Brace:
				switch (c) {
				case '}':
					AddArg();
					st = State::AwaitComma;
					continue;
				case '\\':
					if (i + 1 < scriptLength) {
						switch (text[++i]) {
						case 'n': argss << '\n';	continue;
						case 't': argss << '\t';	continue;
						default: argss << text[i];	continue;
						}
					}
					else argss << c; continue;
				default:
					argss << c;
					continue;
				}

			case State::AwaitComma:
				if (whiteSpace.count(c)) continue;
				switch (c) {
				case ',': st = State::ArgInter; continue;
				case ')': st = State::OptInter; continue;
				default: SCRIPT_ERROR("Expected ',' or ')' (after } or '\"'");
				}
				continue;

			case State::Opt:
				if (whiteSpace.contains(c)) {
					AddOpt();
					st = State::OptInter;
					continue;
				}
				switch (c) {
				case ';':
					AddStmt();
					continue;
				default:
					optss << c;
					continue;
				}

			case State::OptInter:
				if (whiteSpace.count(c)) continue;
				switch (c) {
				case ';':
					AddStmt();
					continue;
				default:
					st = State::Opt;
					optss << c;
					continue;
				}
			}
		}

		// Ensure no ending errors:
		std::size_t i = scriptLength - 1;
		char c = text[i];
		switch (st) {
		case State::CmtBlock:
			SCRIPT_ERROR("End of script, unclosed block comment: Expected '#'.");
		case State::Cmd:
		case State::CmdInter:
			SCRIPT_ERROR("End of script, unclosed command head: Expected '(' or ';'.");
		case State::ArgInter:
		case State::Free:
		case State::AwaitComma:
			SCRIPT_ERROR("End of script, unclosed arguments: Expected ')'.");
		case State::Quote:
			SCRIPT_ERROR("End of script, unclosed quotes: Expected '\"'.");
		case State::Brace:
			SCRIPT_ERROR("End of script, unclosed braces: Expected '}'.");
		case State::Opt:
		case State::OptInter:
			SCRIPT_ERROR("End of script, unclosed command: Expected ';'.");
		default: break;
		}

		// No adding in things at the last minute, just put ';' at the end.
		return r;
	}
}
