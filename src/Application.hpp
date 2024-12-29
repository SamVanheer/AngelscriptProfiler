#pragma once

#include <memory>
#include <span>
#include <sstream>
#include <string>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>

#include "imgui/imgui.h"
#include "implot/implot.h"

class ScriptingSystem;

class Application final
{
public:
	Application();
	~Application();

	int Run(std::span<const char*> args);

private:
	bool Initialize();

	void Shutdown();

	void RunProgram();

	void UpdateProgram();

	void Update();
	void Render();

	void SetScript(std::string script);

	void ProcessEvents();
	void ProcessEvent(SDL_Event& event);

private:
	SDL_Window* _window{};
	SDL_GLContext _openglContext{};

	ImGuiContext* _imguiContext{};
	ImPlotContext* _implotContext{};

	bool _imSDL2Init{ false };
	bool _imGLInit{ false };

	std::ostringstream _scriptOutput;

	std::unique_ptr<ScriptingSystem> _scriptingSystem;

	bool _running{ false };

	std::string _script;
};
