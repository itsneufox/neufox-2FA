#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <cstdint>
#include <cstddef>

namespace TOTPUtils
{
	void generateSecret(char* output, size_t length);
	bool verifyTOTP(const char* secret, const char* code, uint64_t timestamp, int timeStep = 30, int window = 1);
	void generateTOTP(const char* secret, uint64_t timestamp, char* output, int timeStep = 30);
}
