#pragma once
#include <string>
#include <stdexcept>

namespace CCRepl {
	class ReplUserException : public std::runtime_error {
	public:
		explicit ReplUserException(const std::string& msg) : std::runtime_error(msg) {

		}
	};

	class ReplException : public std::runtime_error {
	public:
		explicit ReplException(const std::string& msg) : std::runtime_error(msg) {

		}
	};

	class ReplCancel : public std::runtime_error {
	public:
		explicit ReplCancel(const std::string& msg = "Cancelled.") : std::runtime_error(msg) {

		}
	};

	class ScriptException : public std::runtime_error {
	public:
		explicit ScriptException(const std::string& msg) : std::runtime_error(msg) {

		}
	};
}