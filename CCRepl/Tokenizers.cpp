#include "pch.h"
#include <CCRepl/Tokenizers.h>
#include <CCRepl/ReplException.h>
#include <util/str.h>
#include <util/fmt.h>
#include <util/parsers.h>

namespace CCRepl {

	std::string CommandTokens::Print() const {
		return str::PresentList(args, commandHead, ", ", "(", ")") + str::PresentList(opts, "", " ", " ", "");
	}

	std::string CommandTokens::Print_ML() const {
		return str::PresentList(args, commandHead, ",\n\t", " (\n\t", "\n)") + str::PresentList(opts, "", " ", " ", "");
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
		cmdStr = str::Trim(cmdStr, '.');
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
	
}