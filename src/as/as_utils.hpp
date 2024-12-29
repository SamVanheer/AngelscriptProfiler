#pragma once

#include <memory>

#include <angelscript.h>

namespace as
{
struct ScriptEngineDeleter
{
	void operator()(asIScriptEngine* engine) const noexcept
	{
		if (engine)
		{
			engine->ShutDownAndRelease();
		}
	}
};

using EnginePtr = std::unique_ptr<asIScriptEngine, ScriptEngineDeleter>;

struct ScriptModuleDeleter
{
	void operator()(asIScriptModule* module) const noexcept
	{
		if (module)
		{
			module->Discard();
		}
	}
};

using ModulePtr = std::unique_ptr<asIScriptModule, ScriptModuleDeleter>;

/**
 *	@brief Custom deleter for Angelscript objects stored in @c std::unique_ptr
 */
template <typename T>
struct ObjectDeleter
{
	void operator()(T* object) const noexcept
	{
		if (object)
		{
			object->Release();
		}
	}
};

template <typename T>
using UniquePtr = std::unique_ptr<T, ObjectDeleter<T>>;

template <typename T>
UniquePtr<T> MakeUnique(T* object)
{
	UniquePtr<T> ptr{object};

	if (ptr)
	{
		ptr->AddRef();
	}

	return ptr;
}
}
