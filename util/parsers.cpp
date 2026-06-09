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
				return tm;
			}
		}

		throw ParseException();
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
}