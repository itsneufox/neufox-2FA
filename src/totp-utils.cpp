/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include "totp-utils.hpp"
#include <array>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

namespace
{
	constexpr std::string_view BASE32_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
	constexpr size_t SECRET_LENGTH = 16;

	std::array<uint8_t, 20> hmacSHA1(const uint8_t* key, size_t keyLen, const std::array<uint8_t, 8>& data)
	{
		std::array<uint8_t, 20> output;
		unsigned int len = 20;
		HMAC(EVP_sha1(), key, keyLen, data.data(), data.size(), output.data(), &len);
		return output;
	}

	std::optional<std::vector<uint8_t>> decodeBase32(const std::string& input)
	{
		std::vector<uint8_t> output;
		uint32_t buffer = 0;
		int bitsLeft = 0;

		for (char c : input)
		{
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '-')
				continue;

			int val = -1;
			if (c >= 'A' && c <= 'Z')
				val = c - 'A';
			else if (c >= '2' && c <= '7')
				val = c - '2' + 26;
			else if (c >= 'a' && c <= 'z')
				val = c - 'a';

			if (val < 0)
				return std::nullopt;

			buffer = (buffer << 5) | val;
			bitsLeft += 5;

			if (bitsLeft >= 8)
			{
				output.push_back(static_cast<uint8_t>((buffer >> (bitsLeft - 8)) & 0xFF));
				bitsLeft -= 8;
			}
		}

		return output;
	}
}

namespace TOTPUtils
{
	std::optional<std::string> generateSecret()
	{
		std::array<uint8_t, SECRET_LENGTH> randomBytes;
		if (RAND_bytes(randomBytes.data(), randomBytes.size()) != 1)
			return std::nullopt;

		std::string secret;
		secret.reserve(SECRET_LENGTH);

		for (uint8_t byte : randomBytes)
			secret += BASE32_CHARS[byte % 32];

		return secret;
	}

	std::string generateTOTP(const std::string& secret, uint64_t timestamp, int timeStep)
	{
		auto key = decodeBase32(secret);
		if (!key)
			return {};

		uint64_t timeCounter = timestamp / timeStep;

		std::array<uint8_t, 8> timeBytes;
		for (int i = 7; i >= 0; i--)
		{
			timeBytes[i] = timeCounter & 0xFF;
			timeCounter >>= 8;
		}

		auto hash = hmacSHA1(key->data(), key->size(), timeBytes);

		int offset = hash[19] & 0x0F;
		uint32_t code = ((hash[offset] & 0x7F) << 24)
		              | ((hash[offset + 1] & 0xFF) << 16)
		              | ((hash[offset + 2] & 0xFF) << 8)
		              | (hash[offset + 3] & 0xFF);

		code = code % 1000000;

		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(6) << code;
		return oss.str();
	}

	bool verifyTOTP(const std::string& secret, const std::string& code, uint64_t timestamp, int timeStep, int window)
	{
		if (secret.empty() || code.length() != 6)
			return false;

		for (char c : code)
		{
			if (c < '0' || c > '9')
				return false;
		}

		for (int i = -window; i <= window; i++)
		{
			uint64_t adjustedTime = timestamp + (i * timeStep);
			std::string expectedCode = generateTOTP(secret, adjustedTime, timeStep);

			if (code == expectedCode)
				return true;
		}

		return false;
	}
}
