#include <algorithm>

#include <angelscript.h>

#include "ScriptProfiler.hpp"

void ScriptProfiler::LineCallback(asIScriptContext* ctx)
{
	auto function = ctx->GetFunction();

	if (!function)
	{
		return;
	}

	int column;
	const char* sectionName;
	int lineNumber = ctx->GetLineNumber(0, &column, &sectionName);

	if (lineNumber <= 0)
	{
		return;
	}

	auto script = _scripts.find(sectionName);

	if (script == _scripts.end())
	{
		script = _scripts.insert_or_assign(sectionName, ScriptProfileData{}).first;
	}

	auto funcData = std::find_if(script->second.Functions.begin(), script->second.Functions.end(), [&](const auto& candidate)
		{
			return candidate.Function.get() == function;
		});

	if (funcData == script->second.Functions.end())
	{
		int row;
		if (function->GetDeclaredAt(nullptr, &row, nullptr) < 0)
		{
			return;
		}

		script->second.Functions.push_back(FunctionProfileData{ .Function{as::MakeUnique(function)}, .FirstLineNumber = row });

		funcData = script->second.Functions.end() - 1;
	}

	// Constructors can have line numbers that precede the first line.
	if (lineNumber < funcData->FirstLineNumber)
	{
		lineNumber = funcData->FirstLineNumber;
	}

	const std::size_t index = static_cast<std::size_t>(lineNumber) - funcData->FirstLineNumber;

	if (index >= funcData->LineDurations.size())
	{
		funcData->LineDurations.resize(index + 1, ProfilerClock::duration::zero());
	}

	const auto currentTime = ProfilerClock::now();

	const auto lineDuration = currentTime - _lastTime;

	_lastTime = currentTime;

	funcData->LineDurations[index] += lineDuration;
}
