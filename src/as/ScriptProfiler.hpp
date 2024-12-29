#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

#include "as_utils.hpp"

class asIScriptContext;
class asIScriptFunction;

using ProfilerClock = std::chrono::high_resolution_clock;

struct FunctionProfileData
{
	as::UniquePtr<asIScriptFunction> Function;
	int FirstLineNumber{};
	std::vector<ProfilerClock::duration> LineDurations;
};

struct ScriptProfileData
{
	std::vector<FunctionProfileData> Functions;
};

class ScriptProfiler
{
public:

	void LineCallback(asIScriptContext* ctx);

	ProfilerClock::time_point GetStartTime() const { return _startTime; }

	void SetStartTime(ProfilerClock::time_point time)
	{
		_startTime = time;
		_lastTime = time;
	}

	ProfilerClock::time_point GetLastTime() const { return _lastTime; }

	const std::unordered_map<std::string, ScriptProfileData>& GetProfileData() const { return _scripts; }

	void Clear()
	{
		_scripts.clear();
	}

private:
	ProfilerClock::time_point _startTime{ ProfilerClock::now() };
	ProfilerClock::time_point _lastTime{ _startTime };

	std::unordered_map<std::string, ScriptProfileData> _scripts;
};
