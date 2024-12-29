#pragma once

#include <memory>
#include <string_view>

#include <angelscript.h>

#include <spdlog/logger.h>

#include "as_utils.hpp"

class ScriptProfiler;

class ScriptingSystem final
{
public:
	explicit ScriptingSystem(std::shared_ptr<spdlog::logger> logger);
	~ScriptingSystem();

	const ScriptProfiler* GetProfiler() const { return _profiler.get(); }

	void SetScript(std::string_view script);

	void Execute();

private:
	void MessageCallback(const asSMessageInfo* msg);

	void RegisterAPI();

	void Print(const std::string& text);

private:
	std::shared_ptr<spdlog::logger> _logger;
	as::EnginePtr _engine;
	as::UniquePtr<asIScriptContext> _context;
	std::unique_ptr<ScriptProfiler> _profiler;
	as::ModulePtr _module;
};
