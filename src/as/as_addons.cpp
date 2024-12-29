/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

// If you need one of the addons included with the Angelscript library itself, add it here by including the .cpp file.
// Note: if you need to modify the code, copy the header and source files and add them to the project directly.

// The include order matters here.

// Avoid conflicts with std::byte
#define _HAS_STD_BYTE 0

#include <angelscript/scriptarray/scriptarray.cpp>
#include <angelscript/scriptbuilder/scriptbuilder.cpp>

// Avoid redefining this to GetObjectA
#undef GetObject

#include <angelscript/scriptdictionary/scriptdictionary.cpp>
#include <angelscript/scriptstdstring/scriptstdstring.cpp>
#include <angelscript/scriptstdstring/scriptstdstring_utils.cpp>
