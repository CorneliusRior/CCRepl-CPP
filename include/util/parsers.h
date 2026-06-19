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
	std::tm PrsDate(const std::string& text);
	std::tm PrsDate(int num);

	bool TryString(const std::string& text, std::string& v);
	bool TryString_ML(const std::string& text, std::string& v);
	bool TryInt(const std::string& text, int& v);
	bool TryDouble(const std::string& text, double& v);
	bool TrySize_t(const std::string& text, std::size_t& v);
	bool TryBool(const std::string& text, bool& v);
	bool TryTime(const std::string& text, std::tm& v);
	bool TryDate(const std::string& text, std::tm& v);
	bool TryIntDate(int num, std::tm& v);

	int ToIntDate(const std::tm& date);	// Returns int which is YYYYMMDD, can be parsed w/ PrsDate(int)
	void ValidateTime(const std::tm& t);	// Throws if invalid. 
	void ValidateDate(const std::tm& t);	// Throws if invalid. Time can be uninitialized
}