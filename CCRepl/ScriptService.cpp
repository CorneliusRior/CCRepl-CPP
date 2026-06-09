#include "pch.h"
#include <CCRepl/ScriptService.h>
#include <util/fmt.h>

namespace CCRepl {

	bool ScriptService::LoadScript(const std::string& fileName) {
		std::filesystem::path filePath = dir_ / fileName;
		try {
			Script s = ScriptReader::ReadFile(filePath, ctx_);
			std::string key = s.MetaData.Name;
			scripts_.insert({ key, std::move(s) });
			return true;
		}
		catch (const ScriptException& ex) {
			ctx_->WriteLine(std::format("Could not parse file '{}': {}", filePath.filename().string(), ex.what()));
			return false;
		}
		catch (const std::exception& ex) {
			ctx_->WriteLine(std::format("Could not load '{}': {}", filePath.filename().string(), ex.what()));
			return false;
		}
	}

	bool ScriptService::LoadScriptAbs(const std::string& filePath) {
		std::filesystem::path pth = filePath;
		try {
			Script s = ScriptReader::ReadFile(pth, ctx_);
			std::string key = s.MetaData.Name;
			scripts_.insert({ key, std::move(s) });
			return true;
		}
		catch (const ScriptException& ex) {
			ctx_->WriteLine(std::format("Could not parse file '{}': {}", pth.filename().string(), ex.what()));
			return false;
		}
		catch (const std::exception& ex) {
			ctx_->WriteLine(std::format("Could not load '{}': {}", pth.filename().string(), ex.what()));
			return false;
		}
	}

	bool ScriptService::LoadAll(const std::string& source) {
		std::filesystem::path src = source.empty() ? dir_ : dir_ / source;
		std::vector<std::filesystem::path> files = ScriptReader::ListFiles(src);
		for (const auto& file : files) {
			try { 
				Script s = ScriptReader::ReadFile(file, ctx_); 
				std::string key = s.MetaData.Name;
				scripts_.insert({ key, std::move(s) });
			}
			catch (const ScriptException& ex) { ctx_->WriteLine(std::format("Skipping file '{}' due to parse error: {}", file.filename().string(), ex.what())); }
			catch (const std::exception& ex) { ctx_->WriteLine(std::format("Skipping file '{}' due to error: {}", file.filename().string(), ex.what())); }
		}
		return true;
	}

	bool ScriptService::LoadAllAbs(const std::string& source) {
		std::vector<std::filesystem::path> files = ScriptReader::ListFiles(source);
		for (const auto& file : files) {
			try {
				Script s = ScriptReader::ReadFile(file, ctx_);
				std::string key = s.MetaData.Name;
				scripts_.insert({ key, std::move(s) });
			}
			catch (const ScriptException& ex) { ctx_->WriteLine(std::format("Skipping file '{}' due to parse error: {}", file.filename().string(), ex.what())); }
			catch (const std::exception& ex) { ctx_->WriteLine(std::format("Skipping file '{}' due to error: {}", file.filename().string(), ex.what())); }
		}
		return true;
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
	
	bool ScriptService::RenameScript(const std::string& oldName, const std::string& newName) {
		if (!HasScript(oldName)) throw ReplUserException(std::format("No loaded script with name '{}'", oldName));
		if (HasScript(newName)) throw ReplUserException(std::format("Cannot rename script to '{}': already exists.", newName));
		auto node = scripts_.extract(oldName);
		node.key() = newName;
		scripts_.insert(std::move(node));
		Script* n = &GetScript(newName);
		n->MetaData.Name = newName;
		return true;
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
	
	std::vector<std::string> ScriptService::ScriptNames() const {
		std::vector<std::string> r; 
		r.reserve(scripts_.size());
		for (const auto& [k, v] : scripts_) r.push_back(k);
		return r;
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

	Script ScriptReader::ReadFile(const std::filesystem::path& path, ReplContext* ctx) {
		return ReadFile(path.string(), ctx);
	}

	Script ScriptReader::ReadFile(const std::string& path, ReplContext* ctx) {
		if (!path.ends_with(".ccr")) throw ReplUserException(std::format("File not of type '.ccr': '{}'", path));
		std::string raw = str::ReadTextFile(path);
		return TextToScript(*ctx, raw);
	}

	std::vector<std::filesystem::path> ScriptReader::ListFiles(const std::filesystem::path& dir) {
		std::vector<std::filesystem::path> r;
		for (const auto& entry : std::filesystem::directory_iterator(dir)) {
			if (!entry.is_regular_file()) continue;
			if (entry.path().extension() != ".ccr") continue;
			r.push_back(entry.path());
		}
		return r;
	}

}