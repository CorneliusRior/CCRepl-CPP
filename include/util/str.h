#pragma once
#include <algorithm>
#include <chrono>
#include <format>
#include <functional>
#include <fstream>
#include <unordered_set>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h> // delete after

#define DB(msg) str::db(msg)

#define STR_P(...) do { \
    std::string file = __FILE__; \
    std::string loc = "(" + file.substr(file.find_last_of('\\') + 1) \
        + " line " + std::to_string(__LINE__) \
        + ", in " + __func__ + "()): "; \
    str::PrintArgs(loc, __VA_ARGS__); \
} while (0)

#define STR_W(msg) str::pol(msg)

#define STR_PRINT_V(vec) \
    str::PresentList(vec, "std::vector<std::string> " #vec " = ", "\",\n  \"", "{\n  \"", "\"\n}")

#define STR_VAR(var) std::format("{} = '{}'", #var, var)
#define STR_VARS(...) STR_P(__VA_ARGS__)
#define STR_VAR_DEF(var) std::format("{} = '{}'", #var, var)
#define STR_FUNC_LOG(...) str::FuncLog(__FILE__, __LINE__, __func__, #__VA_ARGS__, __VA_ARGS__)
#define STR_FUNC_LOG_ML(...) str::FuncLogML(__FILE__, __LINE__, __func__, #__VA_ARGS__, __VA_ARGS__)


namespace str {
	std::size_t StrLength(const std::string& text);
    // Removes the first n letters from string text, accounts for UTF-8 formatting.
    std::size_t Utf8BytePos(const std::string& text, std::size_t charIndex);
    // Returns a substring from text, starting at start, of length 'length', accounts for UTF-8 formatting.
	std::string SubStrUtf8(const std::string& text, std::size_t start, std::size_t length);
    // Returns length of text, accounts for UTF-8 formatting.
    std::string DropFirstUtf8(const std::string& text, std::size_t n);
    // Removes the last n letters from string text, accounts for UTF-8 formatting.
    std::string DropLastUtf8(const std::string& text, std::size_t n);
    // Repeats a char n times.
    std::string Repeat(const char c, std::size_t n);
    // Repeats a string n times.
    std::string Repeat(const std::string& text, std::size_t n);
    // Replaces all new paragraphs with ' '.
    std::string ToSingleLine(const std::string& text, bool removeDuplicates = true);
    // Returns text lower case.
    std::string ToLower(const std::string& text);
    // Returns text as upper case.
    std::string ToUpper(const std::string& text);
    // Sets the first non-whitespace character in the string as upper case.
    std::string Capitalize(const std::string& text);

    // Removes whitespace chars from start and end of text.
    std::string Trim(const std::string& text);
    // Removes instances of char c from start and end of text.
    std::string Trim(const std::string& text, char c);
    // Removes instances of any char in cs from start and end of text.
    std::string Trim(const std::string& text, const std::vector<char>& cs);
    // Removes whitespace chars from start of text.
    std::string TrimStart(const std::string& text);
    // Removes instances of char c from start of text.
    std::string TrimStart(const std::string& text, char c);
    // Removes instances of any char in cs from start of text.
    std::string TrimStart(const std::string& text, const std::vector<char>& cs);
    // Removes whitespace chars from end of text.
    std::string TrimEnd(const std::string& text);
    // Removes instances of char c from end of text.
    std::string TrimEnd(const std::string& text, char c);
    // Removes instances of any char in cs from end of text.
    std::string TrimEnd(const std::string& text, const std::vector<char>& cs);

    // Trims spaces and sets characters to lower case, useful for comparing strings.
    std::string TrimToLower(const std::string& text);
    // Replaces every instance of ' ', ',', '/', or combinations of those with '.' and trims, used for creating command statements.
    std::string DotSeparated(const std::string& text, bool removeDuplicates = true);
    // Sets string to a single line, if length exceeds w, truncates and puts truncateString at the end.
    std::string Truncate(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    // Truncates (see str::Truncate()) then adds spaces to the left side of the string if space remains.
    std::string TruncatePadLeft(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    // Truncates (see str::Truncate()) then adds spaces to the right side of the string if space remains.
    std::string TruncatePadRight(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    // Truncates (see str::Truncate()) then adds remaining spaces on either side of text.
    std::string TruncatePadCenter(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    // Prints every item in a string vector as its own line into a single string.
    std::string PrintList(const std::vector<std::string>& vec);
    // Prints a list with optional title, separation strings, start and end strings.
    std::string PresentList(const std::vector<std::string>& vec, std::string title = "", std::string sep = ", ", std::string start = "[ ", std::string end = " ]");
    // Prints every item in a string vector, separated by string "sep" (default = ", ").
    std::string JoinList(const std::vector<std::string>& vec, const std::string& sep = ", ");
    // Prints every item in a string vector, separated by char "sep" (default = ' ').
    std::string JoinList(const std::vector<std::string>& vec, char sep = ' ');
    // Converts every instance of "\n" in a string to a new line.
    std::string ToMultiLine(const std::string& text);
    // Prints an index line: key to left, truncated by col, value written from col, wraps or truncates at max unless oneline is true.
    std::string ToIndexLine(const std::string& key, const std::string& value, std::size_t col, std::size_t max, bool oneline = true);
    // Returns [x] if true, [ ] if false.
    std::string ToCheckBox(const bool v);
    // Returns percentage of numerator and denominator as a percentage, e.g. 0.5, 2 -> "25%".
    std::string Pct(const double num, const double denom, std::size_t prec = 1);
    // Returns double as a percentage, e.g. 0.25 -> "25%".
    std::string AsPct(const double num, std::size_t prec = 1);
    // Replaces every instance of 'from' in text with 'to'.
    std::string ReplaceAll(const std::string& text, const std::string& from, const std::string& to);

    // Returns true if texts are equal, case insensitive by default.
    bool Equals(const std::string& text1, const std::string& text2, bool caseSensitive = false);
    // Returns true if standard begins with text, case insensitive by default.
    bool StartsWith(const std::string& standard, const std::string& text, bool caseSensitive = false);

    // Returns length of longest string in a string vector.
    std::size_t MaxLength(const std::vector<std::string>& vec);
    // Returns length of longest line in a string.
    std::size_t MaxLength(const std::string& text);
    // returns true if text is in the string vector vec, case insensitive by default.
    bool InVector(const std::string& text, const std::vector<std::string>& vec, bool caseSensitive = false);
    // returns true if c is in the char vector vec, case insensitive by default.
    bool InVector(const char c, const std::vector<char>& vec, bool caseSensitive = false);
    // Split string into a string vector by a character, e.g. str::SplitBy(text, '\n');.
    std::vector<std::string> SplitBy(const std::string& text, char c = ' ');
    // Built a string vector in which every line of v2 is to the right of v1.
    std::vector<std::string> AppendStringVectors(const std::vector<std::string>& v1, const std::vector<std::string>& v2);
    // Add n spaces to the start of every string in vec.
    std::vector<std::string> Indent(const std::vector<std::string>& vec, size_t n = 1);
    // Add n spaces to the start of every line in a string.
    std::vector<std::string> IndentStr(const std::string& text, size_t n = 1);
    // Wrap text into a certain width. Pad adds white space to the end so that every line is the same width.
    std::vector<std::string> Wrap(const std::string& text, std::size_t w, bool pad = false);
    // Prints an index line: key to left, truncated by col, value written from col, wraps or truncates at max unless oneline is true. Returned as a vector.
    std::vector<std::string> ToIndexLineV(const std::string& index, const std::string& value, std::size_t col, std::size_t total, bool oneline = true);

    // Prints a double as a string with prec decimal places. Compact truncates like 1,234,567 -> 1.2M
    std::string ToString(const double value, std::size_t prec = 1, bool compact = false);

    // Time Functions:
    std::string ToString(const std::tm& time);
    std::string ToDateString(const std::tm& time);
    std::string DayOfWeek(const std::tm& time);

    // File things:
    std::string ReadTextFile(const std::string& path);

    void p(std::string text);
    void pol(std::string text);
    void pol(char c);
    void db(std::string text);
    void db(char c_);

    std::vector<std::string> SepArgNames(std::string argNames);

    namespace detail {
        template<typename F>
        static std::string ApplyTrim(const std::string& text, F predicate, int mode = 0) {
            // Mode: Negative = trim start, Positive = trim end, 0 = trim both.
            auto s = mode > 0 ? text.begin() : std::find_if_not(text.begin(), text.end(), predicate);
            auto e = mode < 0 ? text.end() : std::find_if_not(text.rbegin(), text.rend(), predicate).base();
            return (s >= e) ? "" : std::string(s, e);
        }
    }

    /// <summary>
    /// TruncateList: prints the first 'start' items and the last 'end' items, trunating. Used for displaying large amounts of data.
    /// This is for types which can be fed into ostream directly (os << item).
    /// </summary>
    /// <typeparam name="Itm">Item type, which can be directly fed into ostream without conversion.</typeparam>
    /// <param name="items">Vector of items to display.</param>
    /// <param name="itemName">Plural name of individual items used to print amount.</param>
    /// <param name="start">Number of items to display at the start (default = 1).</param>
    /// <param name="end">Number of items to display at the end (default = 1).</param>
    /// <returns></returns>
    template<typename Itm>
    std::string TruncateList(const std::vector<Itm>& items, std::string itemName, std::size_t start = 1, std::size_t end = 1) {
        std::ostringstream oss;
        if (items.size() < start + end + 3) for (const Itm& item : items) oss << item << '\n';
        else {
            for (std::size_t i = 0; i < start; i++) oss << items[i] << '\n';
            oss << "…\n(" << items.size() << " total " << itemName << ")\n…";
            for (std::size_t i = items.size() - end; i < items.size(); i++) oss << '\n' << items[i];
        }
        return oss.str();
    }

    /// <summary>
    /// TruncateList: prints the first 'start' items and the last 'end' items, trunating. Used for displaying large amounts of data.
    /// For types which require conversion into string.
    /// </summary>
    /// <typeparam name="Itm">Item type.</typeparam>
    /// <param name="items">Vector of items to display.</param>
    /// <param name="toStringFunc">Function which returns string from const Itm&.</param>
    /// <param name="itemName">Plural name of individual items used to print amount.</param>
    /// <param name="start">Number of items to display at the start (default = 1).</param>
    /// <param name="end">Number of items to display at the end (default = 1).</param>
    /// <returns></returns>
    template<typename Itm>
    std::string TruncateList(const std::vector<Itm>& items, std::function<std::string(const Itm&)> toStringFunc, std::string itemName, std::size_t start = 1, std::size_t end = 1) {
        std::ostringstream oss;
        if (items.size() < start + end + 3) for (const Itm& item : items) oss << toStringFunc(item) << '\n';
        else {
            for (std::size_t i = 0; i < start; i++) oss << toStringFunc(items[i]) << '\n';
            oss << "…\n(" << items.size() << " total " << itemName << ")\n…";
            for (std::size_t i = items.size() - end; i < items.size(); i++) oss << '\n' << toStringFunc(items[i]);
        }
        return oss.str();
    }

    template<typename... Args>
    void PrintArgs(const std::string& location, const Args&... args) {
        std::ostringstream oss;
        oss << location;
        bool first = true;
        ((oss << (first ? "" : ", ") << args, first = false), ...);
        str::p(oss.str());
    }

    template<typename... Args>
    std::string FuncLog(std::string file, int line, std::string functionName, std::string argNames, Args&&... args) {
        std::ostringstream oss;
        oss << "(" << file.substr(file.find_last_of('\\') + 1) << " line " << line << ") Function called: " << functionName << "(";
        std::vector<std::string> names = SepArgNames(argNames);

        std::size_t i = 0;
        auto printArg = [&](const auto& value) {
            if (i < names.size()) oss << names[i] << " = '" << value << '\'';
            else oss << value;
            if (++i < sizeof...(Args)) oss << ", ";
            };

        (printArg(args), ...);
        oss << ");\n";
        return oss.str();
    }    

    template <typename... Args>
    std::string FuncLogML(std::string file, int line, std::string functionName, std::string argNames, Args&&... args) {
        std::ostringstream oss;
        oss << "(" << file.substr(file.find_last_of('\\') + 1) << " line " << line << ") Function called:\n" << functionName << "(";
        std::vector<std::string> names = SepArgNames(argNames);

        std::size_t i = 0;
        auto printArg = [&](const auto& value) {
            oss << "\n    ";
            if (i < names.size()) oss << names[i] << " = '" << value << '\'';
            else oss << '\'' << value << '\'';
            if (++i < sizeof...(Args)) oss << ", ";
            };
        (printArg(args), ...);

        oss << "\n);\n";
        return oss.str();
    }

    template<typename T>
    std::string TypeString() { return typeid(T).name(); };
    
    template<>
    inline std::string TypeString<bool>() { return "bool"; }

    template<>
    inline std::string TypeString<int>() { return "int"; }

    template<>
    inline std::string TypeString<double>() { return "double"; }

    template<>
    inline std::string TypeString<std::size_t>() { return "size_t"; }

    template<>
    inline std::string TypeString<std::string>() { return "string"; }
    
    template<>
    inline std::string TypeString<std::tm>() { return "time"; }
}

/*
Adding a new type in this.
 - 1: Add new template for template<> TypeString (above).
 - 2: Add new ToString() function (above & in str.cpp).
 - 3: Add a Prs and Try function in Persers.h & .cpp.
 - 4: Add a new CmdArg function in CommandBuilder.
*/

/* 

    ┌─┬─┐ 218 196 194 196 191  
    │ │ │ 179     179     179
    ├─┼─┤ 195 196 197 196 180  
    │ │ │ 179     179     179
    └─┴─┘ 192 196 193 196 217  

    ╔═╦═╗ 201 205 203 205 187  
    ║ ║ ║ 186     186     186
    ╠═╬═╣ 204 205 206 205 185  
    ║ ║ ║ 186     186     186
    ╚═╩═╝ 200 205 202 205 188  

*/

/*
p(std::format("\n\n████████████████\n█HERE HERE HERE█\n█VVVV VVVV VVVV█\n████████████████\n\n"));
p(std::format("████████████████\n█^^^^ ^^^^ ^^^^█\n█HERE HERE HERE█\n████████████████\n\n"));
*/