#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <limits>
#include <print>
#include <stdexcept>
#include <string_view>

#include <Windows.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ostream_sink.h>

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl2.h"

#include "implot/implot.h"

#include "Application.hpp"
#include "as/ScriptingSystem.hpp"
#include "as/ScriptProfiler.hpp"

const std::string_view Script
{
	R"|(
void Test()
{
	Print("Test");
	Print("foo");
	Print("bar");

	array<string> list;
	array<string> list2 = {"foo"};
	array<string> list3(3, "bar");
	array<string> list4;

	for (uint i = 0; i < 10; ++i)
	{
		list4.insertLast("baz" + formatInt(i + 1));
	}

	array<string> list5 = list4;

	list5.insertLast("last");

	list4 = list5;

	PrintArray(@list);
	PrintArray(@list2);
	PrintArray(@list3);
	PrintArray(@list4);

	Foo f("Hello");

	Foo@ f2 = @f;

	Print(f.Baz);

	ChangeHandle(f2);

	Print(f.Baz);
}

void PrintArray(const array<string>@ list)
{
	for (uint i = 0; i < list.length(); ++i)
	{
		Print(list[i]);
	}
}

class Foo
{
	private string Bar;

	const string& Baz
	{
		get const
		{
			return Bar;
		}
	}

	Foo(const string& in bar)
	{
		Bar = bar;
	}
}

void ChangeHandle(Foo@ f)
{
	Foo foo("World");

	f = foo;
}
)|"
};

Application::Application() = default;
Application::~Application() = default;

int Application::Run(std::span<const char*> args)
{
	const bool success = Initialize();

	if (success)
	{
		RunProgram();
	}

	Shutdown();

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool Application::Initialize()
{
	if (!AllocConsole())
	{
		std::print(stderr, "Couldn't allocate console window\n");
		return false;
	}

	if (!std::freopen("CONIN$", "r", stdin) ||
		!std::freopen("CONOUT$", "w", stdout) ||
		!std::freopen("CONOUT$", "w", stderr))
	{
		// Not sure if this will output anywhere sensible.
		std::print(stderr, "Couldn't redirect standard streams\n");
		return false;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::print(stderr, "Error initializing SDL: {}\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	const SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	_window = SDL_CreateWindow("Angelscript Profiler", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, window_flags);

	if (!_window)
	{
		std::print(stderr, "Error creating SDL Window: {}\n", SDL_GetError());
		return false;
	}

	SDL_MaximizeWindow(_window);

	_openglContext = SDL_GL_CreateContext(_window);

	if (!_openglContext)
	{
		std::print(stderr, "Error creating OpenGL context: {}\n", SDL_GetError());
		return false;
	}

	if (SDL_GL_MakeCurrent(_window, _openglContext) < 0)
	{
		std::print(stderr, "Error making OpenGL context current: {}\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetSwapInterval(1);

	IMGUI_CHECKVERSION();
	_imguiContext = ImGui::CreateContext();
	_implotContext = ImPlot::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGui::StyleColorsDark();

	_imSDL2Init = ImGui_ImplSDL2_InitForOpenGL(_window, _openglContext);

	if (!_imSDL2Init)
	{
		std::print(stderr, "Error initializing ImGui SDL2 backend\n");
		return false;
	}

	_imGLInit = ImGui_ImplOpenGL3_Init();

	if (!_imGLInit)
	{
		std::print(stderr, "Error initializing ImGui OpenGL backend\n");
		return false;
	}

	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
	auto ostreamSink = std::make_shared<spdlog::sinks::ostream_sink_st>(_scriptOutput);

	const std::string pattern = "[%H:%M:%S.%F] [%l] %v";

	consoleSink->set_pattern(pattern);
	ostreamSink->set_pattern(pattern);

	auto logger = std::make_shared<spdlog::logger>("ScriptingSystem", std::initializer_list<spdlog::sink_ptr>{ consoleSink, ostreamSink });

	try
	{
		_scriptingSystem = std::make_unique<ScriptingSystem>(std::move(logger));
	}
	catch (const std::runtime_error& e)
	{
		std::print(stderr, "Error creating scripting system: {}\n", e.what());
		return false;
	}

	SetScript(std::string{ Script });

	return true;
}

void Application::Shutdown()
{
	_scriptingSystem.reset();

	if (_imGLInit)
	{
		ImGui_ImplOpenGL3_Shutdown();
		_imGLInit = false;
	}

	if (_imSDL2Init)
	{
		ImGui_ImplSDL2_Shutdown();
		_imSDL2Init = false;
	}

	if (_imguiContext)
	{
		ImPlot::DestroyContext(_implotContext);
		_implotContext = nullptr;

		ImGui::DestroyContext(_imguiContext);
		_imguiContext = nullptr;
	}

	if (_openglContext)
	{
		SDL_GL_DeleteContext(_openglContext);
		_openglContext = nullptr;
	}

	if (_window)
	{
		SDL_DestroyWindow(_window);
		_window = nullptr;
	}

	SDL_Quit();

	FreeConsole();
}

void Application::RunProgram()
{
	_running = true;

	while (_running)
	{
		ProcessEvents();
		UpdateProgram();
	}
}

void Application::UpdateProgram()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	Update();
	Render();

	ImGui::Render();

	ImGuiIO& io = ImGui::GetIO();

	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(_window);
}

void Application::Update()
{
	// Nothing
}

void Application::Render()
{
	//ImGui::ShowDemoWindow();
	//ImPlot::ShowDemoWindow();

	if (ImGui::Begin("Profiler"))
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

		const ImVec2 region = ImGui::GetContentRegionAvail();

		if (ImGui::BeginChild("Execution", ImVec2(region.x * 0.5f, region.y * 0.5f)))
		{
			if (ImGui::Button("Execute"))
			{
				_scriptingSystem->Execute();
			}

			if (ImGui::BeginChild("Log output"))
			{
				ImGui::TextWrapped("%s", _scriptOutput.str().c_str());
			}

			ImGui::EndChild();
		}

		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("Durations", ImVec2(0, region.y * 0.5f)))
		{
			for (const auto& script : _scriptingSystem->GetProfiler()->GetProfileData())
			{
				if (ImGui::TreeNodeEx(script.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (const auto& function : script.second.Functions)
					{
						if (ImGui::TreeNode(function.Function->GetDeclaration(true, true, true)))
						{
							for (std::size_t i = 0; const auto& line : function.LineDurations)
							{
								if (const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(line).count(); duration > 0)
								{
									ImGui::Text("%zu: %lld nanoseconds", function.FirstLineNumber + i, duration);
								}

								++i;
							}

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}
			}
		}

		ImGui::EndChild();

		if (ImGui::BeginChild("Script", ImVec2(region.x, region.y * 0.5f)))
		{
			if (ImGui::BeginTable("LineNumbers", 1, ImGuiTableFlags_BordersV | ImGuiTableFlags_NoHostExtendX))
			{
				ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed);

				ImGui::PushStyleVarY(ImGuiStyleVar_CellPadding, 0);

				const std::size_t count = 1 + std::count_if(_script.begin(), _script.end(), [](auto c)
					{
						return c == '\n' ? 1 : 0;
					});

				for (std::size_t i = 0; i < count; ++i)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("%zu", i + 1);
				}

				ImGui::PopStyleVar();

				ImGui::EndTable();
			}

			ImGui::SameLine();

			ImGui::Text("%s", _script.c_str());
		}

		ImGui::EndChild();

		ImGui::PopStyleColor();
	}

	ImGui::End();
}

void Application::SetScript(std::string script)
{
	_script = std::move(script);
	_scriptingSystem->SetScript(_script);
}

void Application::ProcessEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event) != 0)
	{
		ProcessEvent(event);
	}
}

void Application::ProcessEvent(SDL_Event& event)
{
	if (ImGui_ImplSDL2_ProcessEvent(&event))
	{
		return;
	}

	switch (event.type)
	{
	case SDL_QUIT:
		_running = false;
		break;

	case SDL_WINDOWEVENT:
		if (event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(_window))
		{
			_running = false;
		}
		break;
	}
}
