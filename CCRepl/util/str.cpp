#include "pch.h"
#include "str.h"
#include <future>
#include <iostream>

namespace str {

	std::size_t StrLength(const std::string& text) {
		std::size_t i = 0;
		for (unsigned char c : text) if ((c & 0b11000000) != 0b10000000) i++;
		return i;
	}

	std::size_t Utf8BytePos(const std::string& text, std::size_t charIndex) {
		std::size_t bytePos = 0;
		std::size_t charCount = 0;

		while (bytePos < text.size()) {
			if ((static_cast<unsigned char>(text[bytePos]) & 0b11000000) != 0b10000000) {
				if (charCount == charIndex) return bytePos;
				charCount++;
			}
			bytePos++;
		}
		return text.size(); // CharIndex >= length, return end
	}

	std::string SubStrUtf8(const std::string& text, std::size_t start, std::size_t length) {
		std::size_t byteStart = Utf8BytePos(text, start);
		std::size_t byteEnd = Utf8BytePos(text, start + length);
		return text.substr(byteStart, byteEnd - byteStart);
	}

	std::string DropFirstUtf8(const std::string& text, std::size_t n) {
		return text.substr(Utf8BytePos(text, n));
	}

	std::string DropLastUtf8(const std::string& text, std::size_t n) {
		std::size_t len = StrLength(text);
		if (n >= len) return "";
		return text.substr(0, Utf8BytePos(text, len - n));
	}

	std::string Repeat(const char c, std::size_t n) {
		return std::string(n, c);
	}

	std::string Repeat(const std::string& text, std::size_t n) {
		std::string r;
		r.reserve(text.size() * n);
		for (std::size_t i = 0; i < n; i++) r += text;
		return r;
	}

	std::string ToSingleLine(const std::string& text, bool removeDuplicates) {
		std::string r = text;
		std::replace_if(
			r.begin(),
			r.end(),
			[](char c) { return c == '\n' || c == '\r' || c == '\n\r';},
			' '
		);

		if (removeDuplicates)
			r.erase(std::unique(r.begin(), r.end(),
				[](char a, char b) { return a == ' ' && b == ' '; }),
				r.end()
			);

		return r;
	}

	std::string ToLower(const std::string& text) {
		std::string r = text;
		std::transform(r.begin(), r.end(), r.begin(),
			[](unsigned char c) {return std::tolower(c);});
		return r;
	}

	std::string ToUpper(const std::string& text) {
		std::string r = text;
		std::transform(r.begin(), r.end(), r.begin(),
			[](unsigned char c) {return std::toupper(c);});
		return r;
	}

	std::string Capitalize(const std::string& text) {
		std::string r = text;
		std::size_t f = r.find_first_not_of(" \t\n\r");
		if (f == std::string::npos) return r;
		r[f] = std::toupper(static_cast<unsigned char>(r[f]));
		return r;
	}

	std::string Trim(const std::string& text) {
		return detail::ApplyTrim(text, [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }, 0);
	}

	std::string Trim(const std::string& text, char c) {
		return detail::ApplyTrim(text, [c](char i) { return i == c; }, 0);
	}

	std::string Trim(const std::string& text, const std::vector<char>& cs) {
		return detail::ApplyTrim(text, [&cs](char c) { return std::find(cs.begin(), cs.end(), c) != cs.end(); }, 0);
	}

	std::string TrimStart(const std::string& text) {
		return detail::ApplyTrim(text, [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }, -1);
	}

	std::string TrimStart(const std::string& text, char c) {
		return detail::ApplyTrim(text, [c](char i) { return i == c; }, -1);
	}

	std::string TrimStart(const std::string& text, const std::vector<char>& cs) {
		return detail::ApplyTrim(text, [&cs](char c) { return std::find(cs.begin(), cs.end(), c) != cs.end(); }, -1);
	}

	std::string TrimEnd(const std::string& text) {
		return detail::ApplyTrim(text, [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }, 1);
	}

	std::string TrimEnd(const std::string& text, char c) {
		return detail::ApplyTrim(text, [c](char i) { return i == c; }, 1);
	}

	std::string TrimEnd(const std::string& text, const std::vector<char>& cs) {
		return detail::ApplyTrim(text, [&cs](char c) { return std::find(cs.begin(), cs.end(), c) != cs.end(); }, 1);
	}

	// Trims spaces and sets characters to lower case, useful for comparing strings.
	std::string TrimToLower(const std::string& text) {
		return ToLower(Trim(text));
	}

	// Replaces every instance of ' ', ',', '/', or combinations of those with '.' and trims, used for creating command statements.
	std::string DotSeparated(const std::string& text, bool removeDuplicates) {
		std::string r = Trim(text);
		std::unordered_set<char> dots = { '.', ' ', ',', '/' };

		// Replace everything with dots:
		std::replace_if(
			r.begin(),
			r.end(),
			[&dots](char c) { return dots.contains(c); },
			'.'
		);

		// Remove duplicates
		if (removeDuplicates)
			r.erase(std::unique(r.begin(), r.end(),
				[](char a, char b) { return a == '.' && b == '.'; }),
				r.end()
			);

		return r;
	}

	std::string Truncate(const std::string& text, std::size_t w, const std::string& truncateString) {
		std::string r = ToSingleLine(text, false);
		std::size_t len = StrLength(r);
		if (w >= len) return r;
		size_t tl = StrLength(truncateString);
		if (w <= tl) return SubStrUtf8(r, 0, w);
		return SubStrUtf8(r, 0, w - tl) + truncateString;
	}

	std::string TruncatePadLeft(const std::string& text, std::size_t w, const std::string& truncateString) {
		std::string r = Truncate(text, w, truncateString);
		return Repeat(" ", w - StrLength(r)) + r;
	}

	std::string TruncatePadRight(const std::string& text, std::size_t w, const std::string& truncateString) {
		std::string r = Truncate(text, w, truncateString);
		return r + Repeat(" ", w - StrLength(r));
	}

	std::string TruncatePadCenter(const std::string& text, std::size_t w, const std::string& truncateString) {
		std::string r = Truncate(text, w, truncateString);
		std::size_t gap = w - StrLength(r);
		std::size_t half = gap / 2;
		return Repeat(" ", half) + r + Repeat(" ", gap - half);
	}

	std::string PrintList(const std::vector<std::string>& vec) {
		std::ostringstream oss;
		for (std::string l : vec) oss << l << '\n';
		return oss.str();
	}

	std::string PresentList(const std::vector<std::string>& vec, std::string title, std::string sep, std::string start, std::string end) {
		std::ostringstream oss;
		oss << title << start;
		for (size_t i = 0; i < vec.size(); i++) {
			oss << vec[i];
			if (i != vec.size() - 1) oss << sep;
		}
		oss << end;
		return oss.str();
	}

	std::string ToMultiLine(const std::string& text) {
		std::ostringstream oss;
		for (std::size_t i = 0; i < text.size(); i++) {
			if (text[i] == '\\') {
				if (i + 1 < text.size()) {
					if (text[i + 1] == 'n') {
						oss << '\n';
						i++;
						continue;
					}
					else if (text[i + 1] == '\\') {
						if (i + 2 < text.size() && text[i + 2] == 'n') {
							oss << "\\n";
							i += 2;
							continue;
						}
						else {
							oss << "\\\\";
							i++;
							continue;
						}
					}
				}
			}
			oss << text[i];
		}
		return oss.str();
	}

	std::string ToIndexLine(const std::string& key, const std::string& value, std::size_t col, std::size_t max, bool oneline) {
		if (col >= max) throw std::runtime_error("str::ToIndex(): col exceeds total.");
		std::ostringstream oss;

		if (oneline) return TruncatePadRight(key, col) + Truncate(value, max - col);
		else {
			std::vector<std::string> lv = ToIndexLineV(key, value, col, max, false);
			for (std::size_t i = 0; i < lv.size(); i++) {
				oss << lv[i];
				if (i + 1 != lv.size()) oss << '\n';
			}
		}
		return oss.str();
	}

	std::string ToCheckBox(const bool v) {
		return v ? "[x]" : "[ ]";
	}

	std::string Pct(const double num, const double denom, std::size_t prec) {
		return ToString((num / denom) * 100, prec) + "%";
	}

	std::string AsPct(const double num, std::size_t prec) {
		return ToString(num * 100, prec) + "%";
	}

	std::string ReplaceAll(const std::string& text, const std::string& from, const std::string& to) {
		std::string r = text;
		std::size_t pos = 0;
		while ((pos = r.find(from, pos)) != std::string::npos) {
			r.replace(pos, from.size(), to);
			pos += to.size(); // advance past replacement to prevent recursion.
		}
		return r;
	}

	bool Equals(const std::string& text1, const std::string& text2, bool caseSensitive) {
		if (caseSensitive) return text1 == text2;
		return ToLower(text1) == ToLower(text2);
	}

	// Does standard begin with text?
	bool StartsWith(const std::string& standard, const std::string& text, bool caseSensitive) {
		if (caseSensitive) return standard.size() >= text.size() && std::equal(text.begin(), text.end(), standard.begin());
		return standard.size() >= text.size() &&
			std::equal(
				text.begin(), text.end(), standard.begin(),
				[](char a, char b) {
					return std::tolower((unsigned char)a) == std::tolower((unsigned char)b);
				}
			);
	}


	// Vector functions:

	std::size_t MaxLength(const std::vector<std::string>& vec) {
		std::size_t r = StrLength(
			*std::max_element(
				vec.begin(), vec.end(),
				[](const std::string& a, const std::string& b) {
					return StrLength(a) < StrLength(b);
				}
			)
		);
		return r;
	}

	std::size_t MaxLength(const std::string& text) {
		std::vector<std::string> vec = SplitBy(text, '\n');
		return MaxLength(vec);
	}

	bool InVector(const std::string& text, const std::vector<std::string>& vec, bool caseSensitive) {
		if (caseSensitive) return std::any_of(vec.begin(), vec.end(), [&](const std::string& s) { return s == text; });
		else return std::any_of(vec.begin(), vec.end(), [&](const std::string& s) { return ToLower(s) == ToLower(text); });
	}

	bool InVector(const char c, const std::vector<char>& vec, bool caseSensitive) {
		if (caseSensitive) return std::any_of(vec.begin(), vec.end(), [&](const char& vc) { return vc == c; });
		else return std::any_of(vec.begin(), vec.end(), [&](const char& vc) { return std::tolower(vc) == std::tolower(c); });
	}

	std::vector<std::string> SplitBy(const std::string& text, char c) {
		std::vector<std::string> r;
		std::stringstream ss(text);
		std::string s;
		while (std::getline(ss, s, c)) {
			if (!s.empty() && s.back() == '\r') s.pop_back();	// Handles for "\r\n"
			r.push_back(s);
		}
		if (!text.empty() && text.back() == c) r.push_back("");	// Lest text ends w/ delim
		return r;
	}

	std::vector<std::string> AppendStringVectors(const std::vector<std::string>& v1, const std::vector<std::string>& v2) {
		// Find dimensions of both of them:
		std::size_t w1 = MaxLength(v1);
		std::size_t w2 = MaxLength(v2);
		std::size_t l1 = v1.size();
		std::size_t l2 = v2.size();
		std::size_t tl = std::max<size_t>({ l1, l2 });
	
		// Build a new string vector being each line of 1 followed by each line of 2, so long as both exist:
		std::vector<std::string> r;
		for (int i = 0; i < tl; i++) {
			std::string s1 = i >= l1 ? Repeat(" ", w1) : TruncatePadRight(v1[i], w1);
			std::string s2 = i >= l2 ? "" : v2[i];
			r.push_back(s1 + s2);
		}
		return r;
	}

	std::vector<std::string> Indent(const std::vector<std::string>& vec, size_t n) {
		std::size_t l = vec.size();
		std::vector<std::string> v1;
		std::string ind = Repeat(" ", n);
		for (int i = 0; i < l; i++) v1.push_back(ind);
		return AppendStringVectors(v1, vec);
	}

	std::vector<std::string> IndentStr(const std::string& text, size_t n) {
		return Indent(SplitBy(text, '\n'), n);
	}

	// Pad: add empty space to make every line equal to w, useful for AppendStringVectors().
	std::vector<std::string> Wrap(const std::string& text, std::size_t w, bool pad) {
		std::vector<std::string> r;
		if (w < 1) return r;
		if (w == 1) {
			for (char c : text) if (c != '\r' && c != '\n') r.push_back(std::string(1, c));
			return r;
		}

		// Set up oss & helper lambdas;
		std::ostringstream oss;
		auto Reset = [&oss]() {
			oss.str("");
			oss.clear();
			};
		auto FlushLine = [&oss, &r, &Reset, &w]() {
			std::string l = oss.str();
			if (!l.empty()) r.push_back(l);
			Reset();
			};

		// Split into paragraphs and iterate:
		std::vector<std::string> paras = SplitBy(text, '\n');		
		for (std::string& p : paras) {
			// If length does not exceed, just add:
			if (StrLength(p) <= w) {
				r.push_back(p);
				continue;
			}

			// Split into words and iterate:
			std::vector<std::string> words = SplitBy(p, ' ');
			for (std::string& n : words) {
				std::size_t len = StrLength(n);

				// If word exceeds w:
				if (len > w) {
					FlushLine();		// If long enough it needs its own line
					std::string o = n;	// "outstanding"
					while (StrLength(o) > w) {
						std::string c = SubStrUtf8(o, 0, w - 1);
						r.push_back(c + "-");
						o = SubStrUtf8(o, w - 1, StrLength(o) - (w - 1));
					}

					if (!o.empty()) {
						oss << o;
						if (StrLength(oss.str()) < w) oss << " ";
					}
					continue;
				}

				// Otherwise (Normal Words): If no space to type, newline, then type.
				std::string l = oss.str();				// current line
				std::size_t t = StrLength(l);			// current total length
				std::size_t q = len + (t > 0 ? 1 : 0);	// required (t > 0 for adding " ")
				if (q > w - t) { 
					FlushLine(); 
					t = 0; 
				}
				if (t > 0) oss << " ";
				oss << n;
			}
			FlushLine();
			//if (&p != &paras.back()) r.push_back("");
		}
		if (pad) {
			std::vector<std::string> padr;
			for (std::string l : r) padr.push_back(TruncatePadRight(l, w));
			return padr;
		}
		return r;
	}

	// One index entry, returned as vector.
	std::vector<std::string> ToIndexLineV(const std::string& index, const std::string& value, std::size_t col, std::size_t total, bool oneline) {
		if (col >= total) throw std::runtime_error("str::ToIndex(): col exceeds total.");		

		if (oneline) {
			std::vector<std::string> r;
			r.push_back(TruncatePadRight(index, col) + Truncate(value, total - col));
			return r;
		}

		std::vector<std::string> k = Wrap(index, col, true);
		std::vector<std::string> v = Wrap(value, total - col);
		return AppendStringVectors(k, v);
	}

	std::string ToString(double value, std::size_t prec, bool compact) {

		auto format = [prec](double x, char suffix = '\0') {
			const double scale = std::pow(10, static_cast<double>(prec));
			if (x >= 0) x = std::floor(x * scale) / scale;
			else x = std::ceil(x * scale) / scale;

			std::ostringstream oss;
			oss << std::fixed << std::setprecision(prec) << x;
			std::string s = oss.str();
			if (auto dot = s.find('.'); dot != std::string::npos) {
				s.erase(s.find_last_not_of('0') + 1);
				if (!s.empty() && s.back() == '.') s.pop_back();
			}

			if (suffix) s.push_back(suffix);
			return s;
		};

		if (compact) {
			const double abs_val = std::abs(value);
			if (abs_val >= 1e15)	return format( value / 1e15, 'Q');
			if (abs_val >= 1e12)	return format( value / 1e12, 'T');
			if (abs_val >= 1e9)		return format( value / 1e9, 'B');
			if (abs_val >= 1e6)		return format( value / 1e6, 'M');
			if (abs_val >= 1e3)		return format( value / 1e3, 'K');
			return format(value);
		}
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(prec) << value;
		return oss.str();
	}

	// ToString Functions:
	std::string ToString(const std::tm& time) {
		std::tm t = time;
		std::ostringstream oss;
		oss << std::put_time(&t, "%Y-%m-%d %H:%M:%S");
		return oss.str();
	}

	std::string ToDateString(const std::tm& time) {
		std::tm t = time;
		std::ostringstream oss;
		oss << std::put_time(&t, "%Y-%m-%d");
		return oss.str();
	}

	std::string DayOfWeek(const std::tm& time) {
		std::tm t = time;
		std::mktime(&t);
		switch (t.tm_wday) {
		case 0: return "Sunday";
		case 1: return "Monday";
		case 2: return "Tuesday";
		case 3: return "Wednesday";
		case 4: return "Thursday";
		case 5: return "Friday";
		case 6: return "Saturday";
		default: return "Unknown";
		}
	}

	std::string ReadTextFile(const std::string& path) {
		std::ifstream file(path);

		if (!file) throw std::runtime_error("Could not open file: " + path);

		std::stringstream buffer;
		buffer << file.rdbuf();

		return buffer.str();
	}

	void p(std::string text) { 
#ifdef NDEBUG

#else
		std::cout << text << std::endl; 
#endif
	}	

	void pol(std::string text) {
#ifdef NDEBUG

#else
		std::cout << text;
#endif
	}

	void pol(char c) {
#ifdef NDEBUG

#else
		std::cout << c;
#endif
	}

	void db(std::string text) {
		OutputDebugStringA(text.c_str());
	}

	void db(char c) {
		OutputDebugStringA(std::string(1, c).c_str());
	}

	std::vector<std::string> SepArgNames(std::string argNames) {
		std::vector<std::string> r;
		std::stringstream ss(argNames);
		std::string name;
		while (std::getline(ss, name, ',')) {
			if (!name.empty() && name[0] == ' ') name.erase(0, 1);
			r.push_back(name);
		}
		return r;
	}
}