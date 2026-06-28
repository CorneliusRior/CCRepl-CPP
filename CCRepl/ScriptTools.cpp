#include <CCRepl/ScriptTools.h>
#include <CCRepl/Script.h>

namespace CCRepl::ScriptTools {

    std::string ParseUpdate(const std::string& text, std::size_t i) {
        std::ostringstream oss;
        oss << "i == " << i 
            << "\n\033[2m" << text.substr(0, i) 
            << "\033[22;46;30m" << text[i] << "\033[0;2m" 
            << (i < text.size() ? text.substr(i + 1, text.size() - i -1) : "")
            << '\n';
        return oss.str();
    }

    std::string ExpandMacros(const std::string& text) {
        // See coment in the Script.cpp, if not copied here or deleted.
        std::string r = text;
        int iterations = 0;
        std::unordered_set<char> whitespace = { ' ', '\n', '\t', '\r' };

        // Replace macros:
        {
            std::ostringstream oss;
            std::map<std::string, std::string> macros;
            for (std::size_t i = 0; i < r.size(); i++) {
                //STR_P(ParseUpdate(r, i));
                char stop;
                oss << TextUntil(r, i, {'@', '#', '/'}, stop, false, false);
                switch (stop) {
                    case '#':
                    case '/': {
                        SkipComment(r, i);
                        continue;
                    }
                    case '@': {
                        std::string keyWord = TextUntil(r, i, ' ', true, false);
                        if (keyWord == "@MACRO ") {                            
                            // No options for macros (for the time being)
                            i++;
                            std::string macroName = str::TrimStart(TextUntil(r, i, ' ', false, true));
                            std::string endStr = "@END(" + macroName + ")";
                            std::string macroVal = str::Trim(TextUntil(r, i, endStr), {' ', '\n', '\t'});
                            macros[macroName] = macroVal;
                        }
                        else oss << keyWord;
                        continue;
                    }
                    case '\x03': break;
                }
            }
            r = oss.str();
            for (const auto& [name, val] : macros) {
                r = str::ReplaceAll(r, name, val);
            }
        }
        
        while (true) {
            iterations++;
            if (iterations > 1000) throw ScriptException("Exceeded 1000 iterations");
            std::ostringstream oss;
            
            // Expand repeat statements: Build string in oss, if it is equal to r, then we return, otherwise we do it again until no difference is made.
            for (std::size_t i = 0; i < r.size(); i++) {
                char stop;
                oss << TextUntil(r, i, {'@', '#', '/'}, stop, false, false);

                //STR_P(ParseUpdate(r, i));

                switch(stop) {
                    case '#':
                    case '/': {
                        SkipComment(r, i);
                        continue;
                    }
                    case '@': {
                        char endKeyChar;
                        std::string keyWord = TextUntil(r, i, {'(', ' '}, endKeyChar, false, false);
                        if (keyWord == "@REPEAT") {
                            if (endKeyChar != '(') throw ScriptException(std::format("Expand Macros @REPEAT(): Unexpected char '{}', expected '('", endKeyChar));
                            std::string repeatName = TextUntil(r, i, ')', true, true); // (true, true), should be like @REPEAT(name) -> "(name)"
                            std::string endStr = "}@END" + repeatName;

                            std::vector<std::string> varNames;
							std::map<std::string, std::vector<std::string>> repeatVars;

							bool defineVars = true;
                            for (; i < r.size() && defineVars; i++) {
                                char c = r[i];
                                if (whitespace.contains(c)) continue;
                                switch (c) {                                    
                                    case ',': continue;
                                    case '#':
                                    case '/': 
                                        SkipComment(r, i);
                                        continue;
                                    case '[': {
										std::string name = TextUntil(r, i, ']', true, false);
										SkipUntil(r, i, '(', false);
										repeatVars[name] = TokenizeArgs(r, i);
										varNames.push_back(name);
										break;
									}
									case '{': 
										defineVars = false;
										break;
                                    default: 
                                        //STR_P(ParseUpdate(r, i));
                                        throw ScriptException(std::format("Expand Macros @REPEAT(): unexpected '{}'.", c));
                                }
                            } 
                            std::string scope = str::Trim(TextUntil(r, i, endStr), {' ', '\n', '\t'});
                            for (const auto& combo : CartesianProduct(varNames, repeatVars)) {
                                std::string gen = scope;
                                for (const auto& [var, val] : combo)
                                    gen = str::ReplaceAll(gen, var, val);
                                oss << gen << '\n';
                            }
                        }
                    }
                    case '\x03':
                        break;
                }
            }
            if (oss.str() == r) return r;;
            r = oss.str();
        }
    }

    std::vector<ScriptToken> TokenizeScript(const std::string& text) {
        /*
		Rules:
		Comments can be formed as either block comments starting & ending with '#', or a line starting with '//', but only outside of statements.
		Statements start at the first letter of the next one, end end with ';'.
		New paragraphs or '\n' are allowed everywhere, unless we have some reason to believe that it's in a quote or brackets or something, we ignore it.
		Otherwise, we follow the general rules of TokenizaParen.
		Like TokenizeParen, we're not doing UTF-8 stuff, just normal input.

		v1.1: Adding keywords (for now, just @REPEAT).
		Putting "@" in InterStmt will do keyword. Continue until the next ' ', that has to match a keyword.
		To Start off, we'll just have "Repeat", case insensitive.
		*/
        std::vector<ScriptToken> r;
        std::unordered_set<char> whiteSpace = { ' ', '\n', '\t', '\r' };
        std::string exp = ExpandMacros(text);
        for (std::size_t i = 0; i < exp.size(); i++) {
            char c = exp[i];
            if (whiteSpace.contains(c)) continue;
            switch(c) {
                case '/':
                case '#':
                    SkipComment(exp, i);
                    continue;
                case ';': continue; // Could throw, but no reason to.
                default:
                    std::size_t startLine = FindLine(exp, i);
                    CommandTokens tokens = TokenizeCmd(exp, i);
                    r.push_back(ScriptToken{tokens, r.size(), startLine, FindLine(text, i)});
                    continue;
            }
        }
        return r;
    }

    std::string TextUntil(const std::string& text, std::size_t& i, char c, bool includeLast, bool consume) {
        std::ostringstream oss;
        while (i < text.size()) {
            if (text[i] == c) {
                if (includeLast) oss << text[i];
                if (consume && i + 1 < text.size()) i++;
                return oss.str();
            }
            oss << text[i++];
        }
        return oss.str();
    }

    std::string TextUntil(const std::string& text, std::size_t& i, std::vector<char> cs, bool includeLast, bool consume) {
        std::ostringstream oss;
        while (i < text.size()) {
            if (str::InVector(text[i], cs)) {
                if (includeLast)  oss << text[i];
                if (consume && i + 1 < text.size()) i++;
                return oss.str();
            }
            oss << text[i++];
        }
        return oss.str();
    }

    std::string TextUntil(const std::string& text, std::size_t& i, std::vector<char> cs, char& stop, bool includeLast, bool consume) {
        std::ostringstream oss;
        while (i < text.size()) {
            if (str::InVector(text[i], cs)) {
                stop = text[i];
                if (includeLast)  oss << text[i];
                if (consume && i + 1 < text.size()) i++;
                return oss.str();
            }
            oss << text[i++];
        }
        stop = '\x03';
        return oss.str();
    }

    std::string TextUntil(const std::string& text, std::size_t& i, const std::string& stopStr, bool includeLast, bool consume) {
        /* std::ostringstream oss;
        std::size_t req = stopStr.size();
        if (req > text.size()) throw ScriptException("TextUntil(string, size_t, string): stopStr exceeds text in length");
        if (i < req) i = req;
        for (; i <= text.size(); i++) {
            if (text.substr(i-req, req) == stopStr) {
                if (includeLast) oss << stopStr;
                if (!consume) i -= req;
                return oss.str();
            } 
            oss << text[i-req];
        }
        throw ScriptException(std::format("TextUntil(string, size_t, string): StopStr '{}' not found.", stopStr)); */
        std::size_t req = stopStr.size();
        std::size_t start = i;
        if (req > text.size()) throw ScriptException("TextUntil(string, size_t, string): stopStr exceeds text in length");
        while (i <= text.size() - req) {
            if (text.substr(i, req) == stopStr) {
                std::string r;
                if (includeLast) r = text.substr(start, i - start + req);
                else r = text.substr(start, i - start);
                if (consume) i += req;
                return r;
            }
            i++;
        }
        throw ScriptException(std::format("TextUntil(string, size_t, string): StopStr '{}' not found.", stopStr));
    }

    std::string ReadAheadUntil(const std::string& text, const std::size_t i, char c, bool includeLast) {
        std::size_t j = i;
        std::ostringstream oss;
        while (j < text.size()) {
            if (text[j] == c) {
                if (includeLast) oss << text[j];
                return oss.str();
            }
            oss << text[j++];
        }
        return oss.str();
    }

    std::string ReadAheadUntil(const std::string& text, const std::size_t i, std::vector<char> cs, bool includeLast) {
        std::size_t j = i;
        std::ostringstream oss;
        while (j < text.size()) {
            if (str::InVector(text[j], cs, true)) {
                if (includeLast) oss << text[j];
                return oss.str();
            }
            oss << text[j++];
        }
        return oss.str();
    }

    std::string ReadAheadUntil(const std::string& text, const std::size_t i, std::vector<char> cs, char& stop, bool includeLast) {
        std::size_t j = i;
        std::ostringstream oss;
        while (j < text.size()) {
            if (str::InVector(text[j], cs, true)) {
                stop = text[j];
                if (includeLast) oss << text[j];
                return oss.str();
            }
            oss << text[j++];
        }
        stop = '\x03';
        return oss.str();
    }

    std::string ReadAheadUntil(const std::string& text, const std::size_t i, const std::string& stopStr, bool includeLast) {
        std::size_t j = i;
        std::ostringstream oss;
        std::size_t req = stopStr.size();
        if (req < text.size()) throw ScriptException("ReadAheadUntil(string, size_t, string): stopStr exceeds text in length");
        if (j < req) j = req;
        for (; j < text.size(); j++) {
            if (text.substr(j-req, req) == stopStr) {
                if (includeLast) oss << stopStr;
                return oss.str();
            }
            oss << text[j-req];
        }
        throw ScriptException(std::format("ReadAheadUntil(string, size_t, string): StopStr '{}' not found.", stopStr));
    }

    bool SkipUntil(const std::string& text, std::size_t& i, char c, bool consume) {
        while (i++ < text.size()) {
            if (text[i] == c) {
                if (consume && i + 1 < text.size()) i++;
                return true;
            }
        }
        return false; // Hit end of text
    }

    bool SkipUntil(const std::string& text, std::size_t& i, std::vector<char> cs, bool consume) {
        while (i++ < text.size()) {
            if (str::InVector(text[i], cs)) {
                if (consume && i + 1 < text.size()) i++;
                return true;
            }            
        }
        return false;
    }

    bool SkipUntil(const std::string& text, std::size_t& i, std::vector<char> cs, char stop, bool consume) {
        while (i++ < text.size()) {
            if (str::InVector(text[i], cs)) {
                stop = text[i];
                if (consume && i + 1 < text.size()) i++;
                return true;
            }            
        }
        stop = '\x03';
        return false;
    }


    void SkipComment(const std::string& text, std::size_t& i) {
        // block or line comment:
        if (i + 1 >= text.size()) return;
        if (text[i] == '#') {
            i++;
            SkipUntil(text, i, '#', false); // Consume set to false, idea being that you'll skip the last '#' on the next iteration.
        }
        else if (text[i] == '/') {
            if (text[i + 1] == '/') SkipUntil(text, i, '\n', false);
        }
        // Could throw here if you wanted to.
    }

    std::size_t FindLine(const std::string& text, const std::size_t& pos) {
		if (pos > text.size()) throw std::runtime_error(std::format("FindLine: Pos exceeds text length. {} >= {}", pos, text.size()));
		std::size_t line = 1;
		for (std::size_t i = 0; i < text.size() && i <= pos; i++) {
			if (text[i] == '\n') line++;
		}
		return line;
	}

    std::vector<std::string> TokenizeArgs(const std::string& text, std::size_t& i) {
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
	}

    CommandTokens TokenizeCmd(const std::string& text, std::size_t& i) {
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
				case '/':
                case '#': SkipComment(text, i); continue;

				case '(': // Start arguments
					tokens.commandHead = RmDoubleDot(cmdss.str());
					tokens.args = TokenizeArgs(text, i);
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
                    default: {
                        char optend;
                        tokens.opts.push_back(TextUntil(text, i, { ' ', ';' }, optend, false, false));
                        if (optend == ';') return tokens;
                        continue;
                    }					
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
	}

} // namespace CCRepl::ScriptTools