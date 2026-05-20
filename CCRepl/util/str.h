#pragma once
#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <unordered_set>
#include <sstream>
#include <string>
#include <vector>

#define STR_PRINT_V(vec) \
    str::PresentList(vec, "std::vector<std::string> " #vec " = ", "\",\n  \"", "{\n  \"", "\"\n}")

#define STR_VAR_DEF(var) std::format("{} = '{}'", #var, var)
#define STR_FUNC_LOG(...) str::FuncLog(__FILE__, __LINE__, __func__, #__VA_ARGS__, __VA_ARGS__)
#define STR_FUNC_LOG_ML(...) str::FuncLogML(__FILE__, __LINE__, __func__, #__VA_ARGS__, __VA_ARGS__)


namespace str {
	std::size_t StrLength(const std::string& text);
    std::string DropFirstUtf8(const std::string& text, std::size_t n);
    std::string DropLastUtf8(const std::string& text, std::size_t n);
	std::string SubStrUtf8(const std::string& text, std::size_t startPos, std::size_t endPos);
    std::string Repeat(const std::string& text, std::size_t n);
    std::string ToSingleLine(const std::string& text);
    std::string ToLower(const std::string& text);
    std::string ToUpper(const std::string& text);
    std::string Capitalize(const std::string& text);
    std::string Trim(const std::string& text);
    std::string TrimAscii(const std::string& text);
    std::string TrimChar(const std::string& text, char c);
    std::string TrimCharAscii(const std::string& text, char c);
    std::string TrimChars(const std::string& text, const std::vector<char>& cs);
    std::string TrimToLower(const std::string& text);
    std::string DotSeparated(const std::string& text, bool removeDuplicates = true);
    std::string Truncate(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    std::string TruncatePadLeft(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    std::string TruncatePadCenter(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    std::string TruncatePadRight(const std::string& text, std::size_t w, const std::string& truncateString = "…");
    std::string PrintList(const std::vector<std::string>& vec);
    std::string PresentList(const std::vector<std::string>& vec, std::string title = "", std::string sep = ", ", std::string start = "[ ", std::string end = " ]");
    std::string ToMultiLine(const std::string& text);
    std::string ToIndexLine(const std::string& key, const std::string& value, std::size_t col, std::size_t total, bool oneline = true);

    bool Equals(const std::string& text1, std::string& text2, bool caseSensitive = false);
    bool StartsWith(const std::string& standard, const std::string& text, bool caseSensitive = false);

    std::size_t MaxLength(const std::vector<std::string>& vec);
    bool InVector(const std::string& text, const std::vector<std::string>& vec, bool caseSensitive = false);
    std::vector<std::string> SplitBy(const std::string& text, char c = ' ');
    std::vector<std::string> AppendStringVectors(const std::vector<std::string>& v1, const std::vector<std::string>& v2);
    std::vector<std::string> Indent(const std::vector<std::string>& vec, size_t n = 1);
    std::vector<std::string> IndentStr(const std::string& text, size_t n = 1);
    std::vector<std::string> Wrap(const std::string& text, std::size_t w, bool pad = false);
    std::vector<std::string> ToIndexLineV(const std::string& index, const std::string& value, std::size_t col, std::size_t total, bool oneline = true);

    std::string ToString(const double value, std::size_t prec = 1, bool compact = false);

    // Time Functions:
    std::string ToString(const std::tm& time);
    std::string ToDateString(const std::tm& time);
    std::string DayOfWeek(const std::tm& time);

    // File things:
    std::string ReadTextFile(const std::string& path);

    void p(std::string text);

    std::vector<std::string> SepArgNames(std::string argNames);

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