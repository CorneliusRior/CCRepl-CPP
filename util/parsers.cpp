#include "pch.h"
#include <util/parsers.h>

namespace parsers {

	// Do:
	std::string PrsString(const std::string& text) {
		return text;
	}

	std::string PrsString_ML(const std::string& text) {
		return str::ToMultiLine(text);
	}

	int PrsInt(const std::string& text) {
		return std::stoi(text);
	}

	double PrsDouble(const std::string& text) {
		return std::stod(text);
	}

	std::size_t PrsSize_t(const std::string& text) {
		std::size_t r = 0;
		auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), r);
		if (ec != std::errc() || ptr != text.data() + text.size()) throw ParseException();
		return r;
	}

	bool PrsBool(const std::string& text) {
		static const std::unordered_set<std::string> yStrs = {
			"y", "yes", "confirm", "positive", "pos", "+", "affirmative", "affirm", "permit", "true", "t", "1"
		};
		static const std::unordered_set<std::string> nStrs = {
			"n", "no", "deny", "negative", "neg", "-", "false", "f", "0"
		};
		std::string t = str::ToLower(text);
		if (yStrs.contains(t)) return true;
		if (nStrs.contains(t)) return false;
		throw ParseException();
	}

	std::tm PrsTime(const std::string& text) {
		const char* formats[] = {
			"%Y-%m-%d %H:%M:%S",
			"%Y/%m/%d %H:%M:%S",
			"%Y-%m-%d",
			"%Y/%m/%d",
			"%H:%M:%S",
			"%H:%M"
		};

		for (const char* fmt : formats) {
			std::tm tm{};
			std::istringstream iss(text);
			iss >> std::get_time(&tm, fmt);
			if (!iss.fail()) {
				tm.tm_isdst = -1;
				std::mktime(&tm);
				ValidateTime(tm);
				return tm;
			}
		}

		throw ParseException();
	}

	// YMD
	std::tm PrsDate(const std::string& text) {
		const char* formats[] = {
			"%Y-%m-%d",
			"%Y/%m/%d"
		};

		for (const char* fmt : formats) {
			std::tm tm{};
			std::istringstream iss(text);
			iss >> std::get_time(&tm, fmt);
			if (!iss.fail()) {
				tm.tm_isdst = -1;
				std::mktime(&tm);
				return tm;
			}
		}

		throw ParseException();
	}

	// Stored as YYYYMMDD
	std::tm PrsDate(int num) {
		std::tm r{};
		r.tm_year = num / 10000 - 1900;
		r.tm_mon = (num / 100) % 100 - 1;
		r.tm_mday = num % 100;
		ValidateDate(r);
		return r;
	}

	// Try:
	bool TryString(const std::string& text, std::string& v) {
		// Silly but makes things easier:
		v = text;
		return true;
	}

	bool TryString_ML(const std::string& text, std::string& v) {
		v = PrsString_ML(text);
		return true;
	}

	bool TryInt(const std::string& text, int& v) {
		try {
			v = PrsInt(text);
			return true;
		}
		catch (...) { return false; }
	}

	bool TryDouble(const std::string& text, double& v) {
		try {
			v = PrsDouble(text);
			return true;
		}
		catch (...) { return false; }
	}

	bool TrySize_t(const std::string& text, std::size_t& v) {
		try {
			v = PrsSize_t(text);
			return true;
		}
		catch (const ParseException&) { return false; }
	}

	bool TryBool(const std::string& text, bool& v) {
		try {
			v = PrsBool(text);
			return true;
		}
		catch (const ParseException&) { return false; }
	}

	bool TryTime(const std::string& text, std::tm& v) {
		try {
			v = PrsTime(text);
			return true;
		}
		catch (const ParseException&) { return false; }
	}

	bool TryDate(const std::string& text, std::tm& v) {
		try {
			v = PrsDate(text);
			return true;
		}
		catch (const ParseException&) { return false; }
	}

	bool TryDate(int num, std::tm& v) {
		try {
			v = PrsDate(num);
			return true;
		}
		catch (const ParseException&) { return false; }
	}

	int ToIntDate(const std::tm& t) {
		return ((t.tm_year + 1900) * 10000) + ((t.tm_mon + 1) * 100) + (t.tm_mday);
	}

	void ValidateTime(const std::tm& t) {
		std::tm check = t;
		std::time_t tt = std::mktime(&check);
		if (tt == -1 || 
			check.tm_year != t.tm_year || 
			check.tm_mon != t.tm_mon ||
			check.tm_mday != t.tm_mday ||
			check.tm_hour != t.tm_hour ||
			check.tm_min != t.tm_min ||
			check.tm_sec != t.tm_sec) 
			throw ParseException();
	}

	void ValidateDate(const std::tm& t) {
		std::tm check = t;
		check.tm_hour = 12;
		check.tm_min = 0;
		check.tm_sec = 0;

		std::time_t tt = std::mktime(&check);
		if (tt == -1 || 
			check.tm_year != t.tm_year || 
			check.tm_mon != t.tm_mon ||
			check.tm_mday != t.tm_mday) 
			throw ParseException();
	}
}