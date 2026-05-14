#include "pch.h"
#include "Tokenizers.h"

namespace CCRepl {

	std::string CommandTokens::Print() const {
		return str::PresentList(args, commandHead, ", ", "(", ")") + str::PresentList(opts, "", " ", " ", "");
	}

	std::string CommandTokens::Print_ML() const {
		return str::PresentList(args, commandHead, ",\n\t", " (\n\t", "\n)") + str::PresentList(opts, "", " ", " ", "");
	}

	std::string ScriptStatement::Print() const {
		return std::format("Statement #{} (Lines {}-{}): {};", stmtIndex, startLine, endLine, tokens.Print());
	}

	std::string ScriptStatement::Print_ML() const {
		return std::format("Statement #{} (Lines {}-{}):\n{};", stmtIndex, startLine, endLine, tokens.Print_ML());
	}

	std::string PrintSStmtList(const std::vector<ScriptStatement>& lst) {
		std::ostringstream oss;
		for (ScriptStatement stmt : lst) {
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
			text = str::TrimChars(text, colons);
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
		fmt::TextBlock blk;
		blk.Lines.push_back(fmt::TextLine(std::format("Format: {}", Format)));
		blk.Lines.push_back(fmt::TextLine(std::format("Script Name: {}", Name)));
		blk.Lines.push_back(fmt::TextLine(std::format("Author: {}", Author)));
		blk.Lines.push_back(fmt::TextLine(std::format("Created: {}", str::ToDateString(Created))));
		return fmt::TextBox(blk, Name).PrintStr();
	}

	CommandTokens TokenizeParen(const std::string& input) {

		std::vector<std::string> args;
		std::vector<std::string> opts;

		// Find commandhead:
		std::size_t cmdLen = input.find('(');
		if (cmdLen == std::string::npos) cmdLen = input.size();
		std::string cmdStr = str::Trim(input.substr(0, cmdLen));
		std::unordered_set<char> dots = { '.', ' ', ',', '/' };	// Replaced by '.' 
		std::replace_if(cmdStr.begin(), cmdStr.end(), [&](char c) { return dots.contains(c); }, '.');
		cmdStr = str::TrimCharAscii(cmdStr, '.');
		cmdStr.erase(std::unique(
			cmdStr.begin(), cmdStr.end(),
			[](char a, char b) {
				return a == '.' && b == '.';
			}),
			cmdStr.end()
		);

		// If there are aguments present (i.e. at least one '('):
		if (cmdLen < input.size()) {
			// Tokenize:
			enum class State {
				Start,
				Inter,		// Space after commas / command, when unsure what type of arg.
				Free,
				Quote,
				Brace,
				AwaitComma	// After quote or Brace, throws if next is not the end of inp, space, or comma.
			};
			State st = State::Start;

			std::ostringstream oss;
			auto AddToken = [&oss, &args, &st]() {
				args.push_back(oss.str());
				oss.str("");
				oss.clear();
				};

			std::string inp = input.substr(cmdLen + 1);
			bool inArgs = true;
			std::size_t i = 0;
			for (i; i < inp.size() && inArgs; i++) {
				char c = inp[i];
				switch (st) {

				case State::Start: {
					switch (c) {
					case ' ': continue;
					case ',': args.push_back(""); st = State::Inter;	continue;	// Blank argument.
					case ')': inArgs = false;							continue;	// End of arguments + blank.
					case '"': st = State::Quote;						continue;
					case '{': st = State::Brace;						continue;
					default: st = State::Free; oss << c;				continue;
					}
				}

				case State::Inter: {
					switch (c) {
					case ' ': continue;
					case ',': args.push_back("");					continue;	// Blank argument.
					case ')': inArgs = false;						continue;	// End of arguments + blank.
					case '"': st = State::Quote;					continue;
					case '{': st = State::Brace;					continue;
					default: st = State::Free; oss << c;			continue;
					}
				}

				case State::Free: {
					switch (c) {
					case ',': AddToken(); st = State::Inter;		continue;
					case ')': AddToken(); inArgs = false;			continue;	// End of Arguments.
					default: oss << c;								continue;
					}
				}

				case State::Quote: {
					switch (c) {
					case '"': AddToken(); st = State::AwaitComma;	continue;
					case '\\': oss << (i + 1 < inp.size() ? inp[++i] : '\\'); continue;
					default: oss << c; continue;
					}
				}

				case State::Brace: {
					switch (c) {
					case '}': AddToken(); st = State::AwaitComma;	continue;
					case '\\':
						if (i + 1 < inp.size()) {
							switch (inp[++i]) {
							case 'n': oss << '\n';					continue;
							case 't': oss << '\t';					continue;
							default: oss << inp[i];					continue;
							}
						}
						else oss << c; continue;
					default: oss << c; continue;
					}
				}

				case State::AwaitComma: {
					switch (c) {
					case ',': st = State::Inter; continue;
					case ' ': continue;
					case ')': inArgs = false; continue;	// End of arguments. 
					default: throw ReplUserException(std::format("Expected ',' or ')': (pos {} = '{}') '{}'", i, c, inp));
					}
				}
				}
			}

			// Hit ')', add opts with space separation if there's anything more.
			if (++i < inp.size()) {
				std::string optStr = inp.substr(i);
				std::ostringstream optss;
				for (char c : optStr) {
					if (c == ' ') {
						if (!optss.str().empty()) {
							opts.push_back(optss.str());
							optss.str("");
							optss.clear();
						}
					}
					else optss << c;
				}
				if (!optss.str().empty()) opts.push_back(optss.str());
			}
		}

		return CommandTokens{ cmdStr, args, opts };
	}

	std::vector<ScriptStatement> TokenizeScript(const std::string& text) {
		/*
		Rules:
		Comments can be formed as either block comments starting & ending with '#', or a line starting with '//', but only outside of statements.
		Statements start at the first letter of the next one, end end with ';'.
		New paragraphs or '\n' are allowed everywhere, unless we have some reason to believe that it's in a quote or brackets or something, we ignore it.
		Otherwise, we follow the general rules of TokenizaParen.
		Like TokenizeParen, we're not doing UTF-8 stuff, just normal input.
		*/

		std::vector<ScriptStatement> r;
		std::size_t scriptLength = text.size();
		if (scriptLength == 0) return r;

		std::size_t statementIndex = 0;
		std::size_t currentLine = 1;
		CommandTokens currentToken;
		ScriptStatement currentStatement;

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
			currentToken.commandHead = str::TrimChar(cmdss.str(), '.');
			cmdss.str("");
			cmdss.clear();

			if (!argss.str().empty()) AddArg();
			if (!optss.str().empty()) AddOpt();

			currentStatement.tokens = currentToken;
			currentStatement.endLine = currentLine;
			r.push_back(currentStatement);
			
			CommandTokens newTk{};
			ScriptStatement newSt{};
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
					AddArg();
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