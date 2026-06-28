#pragma once
#include <CCRepl/Script.h>
#include <map>
#include <string>
#include <vector>

// More or less just trying to rewrite all of script because it's annoying.

#define SCRIPT_ERROR(msg) \
	throw ScriptException(std::format("Script Error (Line #{}, Char {} '{}'): {}", FindLine(text, i), i, (i < text.size() ? text[i] : '\0'), msg))

namespace CCRepl::ScriptTools {

    struct ExpandedScript {
        std::string text;
        std::vector<std::size_t> sourceLine;
    };

    std::string ParseUpdate(const std::string& text, std::size_t i);
    std::string ExpandMacros(const std::string& text);
    std::vector<ScriptToken> TokenizeScript(const std::string& text);

    // Helper/Skip functions: in format (const std::string& text, std::size_t i, params, bool consome = true):
    
    std::string TextUntil(const std::string& text, std::size_t& i, char c, bool includeLast = false, bool consume = true);
    std::string TextUntil(const std::string& text, std::size_t& i, std::vector<char> cs, bool includeLast = false, bool consume = true);
    std::string TextUntil(const std::string& text, std::size_t& i, std::vector<char> cs, char& stop, bool includeLast = false, bool consume = true);
    std::string TextUntil(const std::string& text, std::size_t& i, const std::string& stopStr, bool includeLast = false, bool consume = true);
    std::string ReadAheadUntil(const std::string& text, const std::size_t i, char c, bool includeLast = false);
    std::string ReadAheadUntil(const std::string& text, const std::size_t i, std::vector<char> cs, bool includeLast = false);
    std::string ReadAheadUntil(const std::string& text, const std::size_t i, std::vector<char> cs, char& stop, bool includeLast = false);
    std::string ReadAheadUntil(const std::string& text, const std::size_t i, const std::string& stopStr, bool includeLast = false);
    bool SkipUntil(const std::string& text, std::size_t& i, char c, bool consume = true);
    bool SkipUntil(const std::string& text, std::size_t& i, std::vector<char> cs, bool consume = true);
    bool SkipUntil(const std::string& text, std::size_t& i, std::vector<char> cs, char stop, bool consume = true);
    bool SkipUntil(const std::string& text, std::size_t& i, const std::string& stopStr, bool consume = true);

    void SkipComment(const std::string& text, std::size_t& i);
    std::size_t FindLine(const std::string& text, const std::size_t& pos);
    std::vector<std::string> TokenizeArgs(const std::string& text, std::size_t& i);
    CommandTokens TokenizeCmd(const std::string& text, std::size_t& i);
    std::vector<std::map<std::string, std::string>> CartesianProduct(const std::vector<std::string>& varNames, const std::map<std::string, std::vector<std::string>>& repeatVars);

} // namespace CCRepl::ScriptTools