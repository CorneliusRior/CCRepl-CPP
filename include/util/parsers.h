#pragma once
#include <chrono>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <util/str.h>

namespace parsers {
	class ParseException : public std::runtime_error {
	public:
		explicit ParseException() : std::runtime_error("Cannot parse.") {

		}
	};

	std::string PrsString(const std::string& text);
	std::string PrsString_ML(const std::string& text);
	int PrsInt(const std::string& text);
	double PrsDouble(const std::string& text);
	std::size_t PrsSize_t(const std::string& text);
	bool PrsBool(const std::string& text);
	std::tm PrsTime(const std::string& text);

	bool TryString(const std::string& text, std::string& v);
	bool TryString_ML(const std::string& text, std::string& v);
	bool TryInt(const std::string& text, int& v);
	bool TryDouble(const std::string& text, double& v);
	bool TrySize_t(const std::string& text, std::size_t& v);
	bool TryBool(const std::string& text, bool& v);
	bool TryTime(const std::string& text, std::tm& v);
}