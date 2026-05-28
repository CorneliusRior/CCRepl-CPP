#pragma once
#include <filesystem>
#include <ranges>
#include "Script.h"

/*
Macros, CCSS stands for "CC(Repl) Script Service".
Can do
auto service = CCSS_GET_SVC();
CCSS_GET_SVC()->ListDir();
CCSS_SET_SVC();
*/

// This is ScriptService
#define CCSS_GET_SVC() ctx.ctx.GetService<CCRepl::ScriptService>()
// Defines shared pointer called svc
#define CCSS_SET_SVC() auto svc = ctx.GetService<CCRepl::ScriptService>()

namespace CCRepl {

	class ScriptService {

	private:
		ReplContext* ctx_;
		std::filesystem::path dir_;
		std::unordered_map<std::string, Script> scripts_;

	public:
		ScriptService() = default;
		ScriptService(ReplContext* ctx, std::filesystem::path dir) : ctx_(ctx), dir_(dir) {}

		void LoadScript(const std::string& name, const std::string& fileName);
		void LoadAll(const std::string& source);
		bool Unload(const std::string& name);
		void UnloadAll();
		std::string ListDir() const;	// Overload which does dir_
		std::string ListDir(const std::filesystem::path& dir) const;

		bool HasScript(const std::string& name) const;		
		std::string ListScripts(const std::string& sk) const;
		std::string PrintScript(const std::string& name) const;
		Script& GetScript(const std::string& name);
		const Script& GetScript(const std::string& name) const;
		bool TestScript(const std::string& name, bool silent = false);
		void ExecuteScript(const std::string& name);

	};

	class ScriptReader {
	public:
		Script ReadFile(const std::filesystem::path& path, ReplContext* ctx) const;
		Script ReadFile(const std::string& path, ReplContext* ctx) const;
		std::vector<std::filesystem::path> ListFiles(const std::filesystem::path& dir);
	};

}