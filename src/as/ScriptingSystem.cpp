#include <stdexcept>
#include <string_view>

#include <angelscript/scriptarray/scriptarray.h>
#include <angelscript/scriptbuilder/scriptbuilder.h>
#include <angelscript/scriptdictionary/scriptdictionary.h>
#include <angelscript/scriptstdstring/scriptstdstring.h>

#include "ScriptingSystem.hpp"
#include "ScriptProfiler.hpp"

ScriptingSystem::ScriptingSystem(std::shared_ptr<spdlog::logger> logger)
	: _logger(std::move(logger))
{
	_engine.reset(asCreateScriptEngine());

	if (!_engine)
	{
		throw std::runtime_error("Error creating Angelscript script engine");
	}

	_engine->SetMessageCallback(asMETHOD(ScriptingSystem, MessageCallback), this, asCALL_THISCALL);

	RegisterAPI();

	_context.reset(_engine->CreateContext());

	if (!_context)
	{
		throw std::runtime_error("Error creating Angelscript context");
	}

	_profiler = std::make_unique<ScriptProfiler>();

	_context->SetLineCallback(asMETHOD(ScriptProfiler, LineCallback), _profiler.get(), asCALL_THISCALL);
}

ScriptingSystem::~ScriptingSystem() = default;

void ScriptingSystem::SetScript(std::string_view script)
{
	_profiler->Clear();

	// Free the old module first because StartNewModule will cause the original to be discarded, causing dangling pointer access!
	_module.reset();

	_module.reset([&, this]() -> asIScriptModule*
		{
			CScriptBuilder scriptBuilder{};

			if (scriptBuilder.StartNewModule(_engine.get(), "Module") < 0)
			{
				return nullptr;
			}

			if (scriptBuilder.AddSectionFromMemory("Script", script.data(), script.size()) < 0)
			{
				return nullptr;
			}

			if (scriptBuilder.BuildModule() < 0)
			{
				return nullptr;
			}

			return scriptBuilder.GetModule();
		}());
}

void ScriptingSystem::Execute()
{
	if (!_module)
	{
		return;
	}

	auto function = _module->GetFunctionByName("Test");

	if (!function)
	{
		return;
	}

	_profiler->SetStartTime(ProfilerClock::now());
	_context->Prepare(function);
	_context->Execute();
}

void ScriptingSystem::MessageCallback(const asSMessageInfo* msg)
{
	const auto level = [&]()
		{
			switch (msg->type)
			{
			case asEMsgType::asMSGTYPE_ERROR:
				return spdlog::level::err;
			case asEMsgType::asMSGTYPE_WARNING:
				return spdlog::level::warn;
			default:
			case asEMsgType::asMSGTYPE_INFORMATION:
				return spdlog::level::info;
			}
		}();

	// The engine will often log information not related to a script by passing an empty section string and 0, 0 for the location.
	// Only prepend this information if it's relevant.
	if (msg->section && '\0' != msg->section[0])
	{
		_logger->log(level, "In section \"{}\" at line {}, column {}: {}", msg->section, msg->row, msg->col, msg->message);
	}
	else
	{
		_logger->log(level, "{}", msg->message);
	}
}

void ScriptingSystem::RegisterAPI()
{
	RegisterStdString(_engine.get());
	RegisterScriptArray(_engine.get(), true);
	RegisterScriptDictionary(_engine.get());
	RegisterStdStringUtils(_engine.get());

	_engine->RegisterGlobalFunction("void Print(const string& in text)", asMETHOD(ScriptingSystem, Print), asCALL_THISCALL_ASGLOBAL, this);
}

void ScriptingSystem::Print(const std::string& text)
{
	_logger->info("{}", text);
}
