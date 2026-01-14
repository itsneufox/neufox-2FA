#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

// Define platform for SA-MP SDK
#if defined(_WIN32) || defined(WIN32)
	#ifndef WIN32
		#define WIN32
	#endif
#elif defined(__linux__) || defined(__linux) || defined(linux)
	#ifndef LINUX
		#define LINUX
	#endif
#elif defined(__FreeBSD__)
	#ifndef FREEBSD
		#define FREEBSD
	#endif
#endif

#include <amx/amx.h>
#include <plugincommon.h>

typedef void (*logprintf_t)(const char* format, ...);

extern void* pAMXFunctions;
extern logprintf_t logprintf;
extern bool isPluginMode;

extern "C" const AMX_NATIVE_INFO native_list[];
