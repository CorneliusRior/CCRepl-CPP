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
				catch (const ReplUserException& ex) { SCP_PRS_ERR(std::format("\033[31User error:\033[1m {}", ex.what())) }
				catch (const ReplException& ex) { SCP_PRS_ERR(std::format("\033[31Repl error:\033[1m {}", ex.what())) }
				catch (const std::runtime_error& ex) { SCP_PRS_ERR(std::format("\033[31Error:\033[1m {}", ex.what())) }
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
		catch (const ReplUserException& ex) { ctx.WriteLine(std::format("User error: {}", ex.what())); }
		catch (const ReplException& ex) { ctx.WriteLine(std::format("Repl Error: {}", ex.what())); }
		catch (const std::runtime_error& ex) { ctx.WriteLine(std::format("Error: {}", ex.what())); }
		catch (...) { ctx.WriteLine("Unknown error."); }
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

	// Implement @Repeat

	/* std::vector<ScriptToken> TokenizeScript(const std::string& text) {
		
		
		std::vector<ScriptToken> r;
		std::size_t scriptLength = text.size();
		if (scriptLength == 0) return r;

		std::unordered_set<char> whiteSpace = { ' ', '\n', '\t', '\r' };

		auto NextChar = [&scriptLength, &text](std::size_t i, char c, std::size_t steps = 1) {
			return i + steps < scriptLength && text[i + steps] == c;
			};

		enum class State {
			InterStmt,		// Between statements (comments & cmd allowed)
			Keyword			// In process of getting it, ends on ' '
		};
		State st = State::InterStmt;

		auto KeywordRepeat = [&](std::size_t& i) {
			enum class RptState {
				Vars,		// Defining variables
				Scope		// Scope what we repeat
			};
			RptState rst = RptState::Vars;

			std::string scope;
			std::size_t scopeStart;
			std::size_t scopeEnd;
			std::vector<std::string> varNames;
			std::map<std::string, std::vector<std::string>> repeatVars;

			// Starts on the space. Make new loop to define the variables:
			for (; i < scriptLength && scope.empty(); i++) {
				char c = text[i];
				switch (rst) {

				case RptState::Vars:
					if (whiteSpace.contains(c)) continue;
					switch (c) {

					case '[': {
						std::string name = StringUntil(text, i, ']', true);
						IgnoreUntil(text, i, '(');
						repeatVars[name] = ScriptTools::TokenizeArgs(text, i);
						varNames.push_back(name);
						break;
					}

					case '{': {
						rst = RptState::Scope;
						scopeStart = i + 1;
						continue;
					}

					case ',': continue;

					default: SCRIPT_ERROR(std::format("Unexpected char: '{}'", c));
					}
					continue;

				case RptState::Scope: {
					if (WhiteSpace.contains(c)) continue;
					if (c == '}') {
						scopeEnd = i;
						scope = text.substr(scopeStart, scopeEnd - scopeStart);
					}
					else ScriptTools::TokenizeCmd(text, i); // Consume a full command: skips over '}' in quotes/braces.
					continue;
				}


				} // end switch (rst)	
			} // end of loop

			// Scope defined now. Expand to a string, then tokenize it:
			std::ostringstream oss;
			for (const auto& combo : CartesianProduct(varNames, repeatVars)) {
				std::string gen = scope;
				for (const auto& [var, val] : combo) {
					gen = str::ReplaceAll(gen, var, val);
				}
				oss << gen;
			}

			// Tokenize (we will just copy main loop for now):
			State tst = State::InterStmt;
			std::string expanded = oss.str();

			for (std::size_t e = 0; e < expanded.size(); e++) {
				char c = expanded[e];
				switch (tst) {

				case State::InterStmt: {
					if (whiteSpace.contains(c)) continue;
					switch (c) {
					case '@':
						tst = State::Keyword;
						continue;
					case '/':
						if (e + 1 >= expanded.size() || expanded[e] != '/') SCRIPT_ERROR("(In expanded): Expected '/' (\"//\" denotes line comment).");
						IgnoreUntil(expanded, e, '\n');
						continue;
					case '#':
						if (!IgnoreUntil(expanded, e, '#')) SCRIPT_ERROR("(In expanded): End of script, unclosed block comment, expected '#'.");
						continue;
					case ';':
						SCRIPT_ERROR("(In expanded): ';': Expected statement.");
					default:
						std::size_t startLine = FindLine(text, i);
						CommandTokens tokens = ScriptTools::TokenizeCmd(expanded, e);
						r.push_back(ScriptToken{
							tokens,
							r.size(),
							startLine,
							startLine
							});
						st = State::InterStmt;
						continue;
					}

				}
				}
			}
			};

		for (std::size_t i = 0; i < scriptLength; i++) {
			char c = text[i];
			
			switch (st) {

			case State::InterStmt: {
				if (whiteSpace.contains(c)) continue;
				switch (c) {
				case '@':
					st = State::Keyword;
					continue;
				case '/':
					if (!NextChar(i, '/')) SCRIPT_ERROR("Expected '/' (\"//\" denotes line comment).");
					IgnoreUntil(text, i, '\n');
					continue;
				case '#':
					if (!IgnoreUntil(text, i, '#')) SCRIPT_ERROR("End of script, unclosed block comment, expected '#'.");
					continue;
				case ';':
					SCRIPT_ERROR("';': Expected statement.");
				default:
					std::size_t startLine = FindLine(text, i);
					CommandTokens tokens = ScriptTools::TokenizeCmd(text, i);
					r.push_back(ScriptToken{
						tokens,
						r.size(),
						startLine,
						FindLine(text, i)
						});
					st = State::InterStmt;
					continue;
				}
			}
			
			case State::Keyword: {
				std::string keyword = str::ToUpper(StringUntil(text, i, ' ', false));

				// If this expands to more than like 3 or 4 keywords, make this a whole switch thing:
				if (keyword == "REPEAT") KeywordRepeat(i);
				else SCRIPT_ERROR("Unknown keyword: " + keyword);
				continue;
			}

			}			
		}
		return r;
	}

	
	
 	std::string StringUntil(const std::string& text, std::size_t& i, char c, bool includeLast) {
		std::ostringstream oss;
		while (i < text.size()) {
			if (text[i] == c) {
				if (includeLast) oss << text[i];
				return oss.str();
			}
			oss << text[i++];
		}		
		return oss.str(); // Hit end of text
	}

	std::string StringUntil(const std::string& text, std::size_t& i, std::vector<char> cs, bool includeLast) {
		std::ostringstream oss;
		while (i < text.size()) {
			if (str::InVector(text[i], cs, false)) {
				if (includeLast) oss << text[i];
				else (i--);
				return oss.str();
			}
			oss << text[i++];
		}
		return oss.str(); // Hit end of text
	}

	// New versions:
	std::string StringUntil(const std::string& text, std::size_t& i, const std::string& stopStr) {
		std::ostringstream oss;
		std::size_t req = stopStr.size();
		if (i < req) i = req;
		for (; i < text.size(); i++) {
			if (text.substr(i-req, req) == stopStr) return oss.str();
			oss << text[i-req];
		}
		SCRIPT_ERROR(std::format("End of script: StopStr '{}' not found.", stopStr));
	}

	std::string TextUntil(const std::string& text, std::size_t& i, std::vector<char> cs, char& stop) {
		std::ostringstream oss;
		while (i < text.size()) {
			if (str::InVector(text[i], cs)) {
				stop = text[i];
				return oss.str();
			} 
			oss << text[i++];
		}
		stop = '\x03';
		return oss.str();
	}
	

	bool IgnoreUntil(const std::string& text, std::size_t& i, char c) {
		while (i++ < text.size()) {
			if (text[i] == c) return true;
		}
		return false; // Hit end of text
	}

	char IgnoreUntil(const std::string& text, std::size_t& i, std::vector<char> cs) {
		while (i++ < text.size()) if (str::InVector(text[i], cs, false)) return text[i];
		return '\x03';
	}

	/* std::vector<std::string> TokenizeArgs(const std::string& text, std::size_t& i) {
		if (++i >= text.size()) SCRIPT_ERROR("End of script, unclosed args.");
		enum class State {
			Inter,
			Free,
			Quote,
			Brace,
			AwaitComma
		};
		State st = State::Inter;

		std::vector<std::string> r;
		std::ostringstream oss;
		auto AddToken = [&oss, &r] {
			r.push_back(oss.str());
			oss.str("");
			oss.clear();
			};

		for (; i < text.size(); i++) {
			char c = text[i];
			
			switch (st) {

			case State::Inter: {
				if (WhiteSpace.contains(c)) continue;
				switch (c) {
				case ')':
					if (!oss.str().empty()) AddToken();
					return r;
				case ',': AddToken();			continue;	// Blank argument;
				case '"': st = State::Quote;	continue;
				case '{': st = State::Brace;	continue;
				default:
					st = State::Free;
					oss << c;
					continue;
				}
			}

			case State::Free: {
				switch (c) {
				case ',':
					// No new lines at start and end of free.
					r.push_back(str::Trim(oss.str(), '\n'));
					oss.str("");
					oss.clear();
					st = State::Inter;
					continue;
				case ')':
					// No new lines at start and end of free.
					r.push_back(str::Trim(oss.str(), '\n'));
					oss.str("");
					oss.clear();
					return r;
				default:
					oss << c;
					continue;
				}
			}

			case State::Quote: {
				switch (c) {
				case '"':
					AddToken();
					st = State::AwaitComma;
					continue;
				case '\\':
					oss << (i + 1 < text.size() ? text[++i] : '\\');
					continue;
				default:
					oss << c;
					continue;
				}
			}

			case State::Brace: {
				switch (c) {
				case '}':
					AddToken();
					st = State::AwaitComma;
					continue;
				case '\\':
					if (i + 1 < text.size()) {
						switch (text[++i]) {
						case 'n': oss << '\n';	continue;
						case 't': oss << '\t';	continue;
						default: oss << text[i];	continue;
						}
					}
					else oss << c; continue;
				default:
					oss << c;
					continue;
				}
			}

			case State::AwaitComma: {
				if (WhiteSpace.contains(c)) continue;
				switch (c) {
				case ',': st = State::Inter; continue;
				case ')':
					if (!oss.str().empty()) AddToken();
					return r;
				default: SCRIPT_ERROR("Expected ',' or ')' (after } or '\"'");
				}
				continue;
			}

			}
		}
		
		// End without closing ')':
		switch (st) {
		case State::Inter:
		case State::Free:
		case State::AwaitComma: SCRIPT_ERROR("End of script, unclosed arguments: Expected ')'.");
		case State::Quote: SCRIPT_ERROR("End of script, unclosed quotes: Expected '\"'.");
		case State::Brace: SCRIPT_ERROR("End of script, unclosed braces: Expected '}'.");
		default: break;
		}
		SCRIPT_ERROR("End of script, unknown error, ArgumentTokenization.");
	} */

	/* CommandTokens TokenizeCmd(const std::string& text, std::size_t& i) {
		enum class State {
			Cmd,
			Opt
		};
		State st = State::Cmd;

		CommandTokens tokens;
		std::ostringstream cmdss;
		std::ostringstream optss;

		auto RmDoubleDot = [](std::string cmd) {
			cmd.erase(std::unique(cmd.begin(), cmd.end(),
				[](char a, char b) { return a == '.' && b == '.'; }),
				cmd.end()
			);
			return str::Trim(cmd, {' ', '.'});
			};

		// Starting on the first non-whitespace one.
		for (; i < text.size(); i++) {
			char c = text[i];
			
			switch (st) {

			case State::Cmd: {
				switch (c) {

				case '.':
				case ' ':
				case '\n': // Dots
				case '\t': cmdss << '.'; continue;

				case '#': // Block comment
					IgnoreUntil(text, i, '#');
					continue;

				case '/': // Line comment if 2, dot if one. 
					if (i + 1 < text.size() && text[i + 1] == '/') IgnoreUntil(text, i, '\n');
					else cmdss << '.';
					continue;

				case '(': // Start arguments
					tokens.commandHead = RmDoubleDot(cmdss.str());
					tokens.args = ScriptTools::TokenizeArgs(text, i);
					st = State::Opt;
					continue;

				case ';': // Command with no arguments.
					tokens.commandHead = RmDoubleDot(cmdss.str());
					return tokens;

				default:
					cmdss << c;
					continue;
				
				}
				continue;
			}

			case State::Opt: {
				switch (c) {
				case '.':
				case ' ':
				case '\n':
				case '\t': continue;
				case ';': return tokens;
				default:
					tokens.opts.push_back(StringUntil(text, i, { ' ', ';' }, false));
					continue;
				}
				continue;
			}

			};
		}

		// End without cosing ';':
		switch (st) {
		case State::Cmd: SCRIPT_ERROR("End of script, unclosed Command: Expected ';'");
		case State::Opt: SCRIPT_ERROR("End of script, unclosed Options: Expected ';'");
		}
		SCRIPT_ERROR("End of script, unknown error (TokenizeCmd).");
	}
 
	
 	std::size_t FindLine(const std::string& text, const std::size_t& pos) {
		if (pos > text.size()) throw std::runtime_error(std::format("FindLine: Pos exceeds text length. {} >= {}", pos, text.size()));
		std::size_t line = 1;
		for (std::size_t i = 0; i < text.size() && i <= pos; i++) {
			if (text[i] == '\n') line++;
		}
		return line;
	}

	std::vector<std::map<std::string, std::string>> CartesianProduct(const std::vector<std::string>& varNames, const std::map<std::string, std::vector<std::string>>& repeatVars) {
		std::vector<std::map<std::string, std::string>> r = {{}}; // one empty combination
		for (const std::string& var : varNames) {
			const std::vector<std::string>& values = repeatVars.at(var);
			std::vector<std::map<std::string, std::string>> next;

			for (const auto& existing : r) {
				for (const std::string& val : values) {
					auto combo = existing;
					combo[var] = val;
					next.push_back(std::move(combo));
				}
			}
			r = std::move(next);
		}
		return r;
	} */
}
