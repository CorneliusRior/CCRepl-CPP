#include "pch.h"
#include "ScriptService.h"

namespace CCRepl {

	void ScriptService::LoadScript(const std::string& name, const std::string& fileName) {
		ScriptReader reader;
		std::filesystem::path filePath = dir_ / fileName;
		scripts_.insert({ name, reader.ReadFile(filePath, ctx_) });
	}

	void ScriptService::LoadAll(const std::string& source) {
		ScriptReader reader;
		std::vector<std::filesystem::path> files = reader.ListFiles(dir_);
		for (const auto& file : files) {
			scripts_.insert({ file.filename().string() , reader.ReadFile(file, ctx_) });
		}
	}

	bool ScriptService::Unload(const std::string& name) {
		return scripts_.erase(name) > 0;
	}

	void ScriptService::UnloadAll() {
		scripts_.clear();
	}

	std::string ScriptService::ListDir() const {
		return ListDir(dir_);
	}

	std::string ScriptService::ListDir(const std::filesystem::path& dir) const {
		ScriptReader reader;
		std::vector<std::filesystem::path> files = reader.ListFiles(dir);
		std::ostringstream oss;
		oss << "All files in /" << dir.filename().string() << "/ (" << files.size() << " total):";
		for (std::filesystem::path file : files) oss << "\n  - " << file.filename().string();
		oss << '\n';

		return oss.str();
	}

	bool ScriptService::HasScript(const std::string& name) const {
		return scripts_.find(name) != scripts_.end();
	}
	
	std::string ScriptService::ListScripts(const std::string& sk) const {
		auto filtered = scripts_ | std::views::filter(
			[&sk](const auto& it) { return str::StartsWith(it.first, sk); }
		);
		fmt::TextTable t = Script::GetTableColumns();
		for (const auto& it : filtered) t << it.second.GetTableRow();
		return t.PrintCompact();
	}
	
	std::string ScriptService::PrintScript(const std::string& name) const {
		return GetScript(name).Print();
	}
	
	Script& ScriptService::GetScript(const std::string& name) {
		auto it = scripts_.find(name);
		if (it == scripts_.end()) throw std::runtime_error("No loaded data for script: " + name);
		return it->second;
	}

	const Script& ScriptService::GetScript(const std::string& name) const {
		auto it = scripts_.find(name);
		if (it == scripts_.end()) throw std::runtime_error("No loaded data for script: " + name);
		return it->second;
	}
	
	bool ScriptService::TestScript(const std::string& name, bool silent) {
		return GetScript(name).Test(*ctx_, silent);
	}
	
	void ScriptService::ExecuteScript(const std::string& name) {
		GetScript(name).Execute(*ctx_);
	}

	Script ScriptReader::ReadFile(const std::filesystem::path& path, ReplContext* ctx) const {
		return ReadFile(path.string(), ctx);
	}

	Script ScriptReader::ReadFile(const std::string& path, ReplContext* ctx) const {
		std::string raw = str::ReadTextFile(path);
		return TextToScript(*ctx, raw);
	}

	std::vector<std::filesystem::path> ScriptReader::ListFiles(const std::filesystem::path& dir) {
		std::vector<std::filesystem::path> r;
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
			// if .txt here.
			r.push_back(entry.path());
		}
		return r;
	}

}