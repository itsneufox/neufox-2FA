/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include "totp-utils.hpp"
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <random>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

// Base32 character set (RFC 4648)
static const char BASE32_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

// HMAC-SHA1 implementation using OpenSSL
namespace
{
	void hmacSHA1(const uint8_t* key, size_t keyLen, const uint8_t* data, size_t dataLen, uint8_t* output)
	{
		unsigned int len = 20;
		HMAC(EVP_sha1(), key, keyLen, data, dataLen, output, &len);
	}

	// Decode base32 string to bytes
	bool decodeBase32(const char* input, uint8_t* output, size_t* outputLen)
	{
		size_t inputLen = strlen(input);
		*outputLen = 0;

		uint32_t buffer = 0;
		int bitsLeft = 0;

		for (size_t i = 0; i < inputLen; i++)
		{
			char c = input[i];
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
				return false;

			buffer = (buffer << 5) | val;
			bitsLeft += 5;

			if (bitsLeft >= 8)
			{
				output[(*outputLen)++] = (buffer >> (bitsLeft - 8)) & 0xFF;
				bitsLeft -= 8;
			}
		}

		return true;
	}
}

namespace TOTPUtils
{
	void generateSecret(char* output, size_t length)
	{
		if (length < 17)
			return;

		uint8_t randomBytes[16];
		if (RAND_bytes(randomBytes, sizeof(randomBytes)) != 1)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, 31);
			for (size_t i = 0; i < 16; i++)
			{
				output[i] = BASE32_CHARS[dis(gen)];
			}
		}
		else
		{
			for (size_t i = 0; i < 16; i++)
				output[i] = BASE32_CHARS[randomBytes[i] % 32];
		}
		output[16] = '\0';
	}

	void generateTOTP(const char* secret, uint64_t timestamp, char* output, int timeStep)
	{
		uint8_t key[64];
		size_t keyLen;
		if (!decodeBase32(secret, key, &keyLen))
		{
			output[0] = '\0';
			return;
		}

		uint64_t timeCounter = timestamp / timeStep;

		uint8_t timeBytes[8];
		for (int i = 7; i >= 0; i--)
		{
			timeBytes[i] = timeCounter & 0xFF;
			timeCounter >>= 8;
		}

		uint8_t hash[20];
		hmacSHA1(key, keyLen, timeBytes, 8, hash);

		int offset = hash[19] & 0x0F;
		uint32_t code = ((hash[offset] & 0x7F) << 24)
		              | ((hash[offset + 1] & 0xFF) << 16)
		              | ((hash[offset + 2] & 0xFF) << 8)
		              | (hash[offset + 3] & 0xFF);

		code = code % 1000000;
		sprintf(output, "%06u", code);
	}

	bool verifyTOTP(const char* secret, const char* code, uint64_t timestamp, int timeStep, int window)
	{
		if (!secret || !code || strlen(code) != 6)
			return false;

		for (int i = 0; i < 6; i++)
		{
			if (code[i] < '0' || code[i] > '9')
				return false;
		}

		for (int i = -window; i <= window; i++)
		{
			uint64_t adjustedTime = timestamp + (i * timeStep);
			char expectedCode[7];
			generateTOTP(secret, adjustedTime, expectedCode, timeStep);

			if (strcmp(code, expectedCode) == 0)
				return true;
		}

		return false;
	}
}
