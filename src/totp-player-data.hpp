#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <chrono>
#include <cstring>

constexpr size_t TOTP_SECRET_LENGTH_SAMP = 16;
constexpr int MAX_PLAYERS = 1000;

struct PlayerTOTPData
{
	bool enabled;
	bool verified;
	char secret[TOTP_SECRET_LENGTH_SAMP + 1];
	int failedAttempts;
	std::chrono::steady_clock::time_point lastAttempt;

	PlayerTOTPData()
		: enabled(false)
		, verified(false)
		, failedAttempts(0)
		, lastAttempt(std::chrono::steady_clock::time_point::min())
	{
		secret[0] = '\0';
	}

	void reset()
	{
		enabled = false;
		verified = false;
		secret[0] = '\0';
		failedAttempts = 0;
		lastAttempt = std::chrono::steady_clock::time_point::min();
	}

	bool hasSecret() const
	{
		return secret[0] != '\0';
	}

	void setSecret(const char* newSecret)
	{
		if (newSecret)
		{
			strncpy(secret, newSecret, TOTP_SECRET_LENGTH_SAMP);
			secret[TOTP_SECRET_LENGTH_SAMP] = '\0';
		}
		else
		{
			secret[0] = '\0';
		}
	}
};

class PlayerDataManager
{
private:
	PlayerTOTPData players_[MAX_PLAYERS];
	static PlayerDataManager* instance_;

	PlayerDataManager() = default;

public:
	static PlayerDataManager& Get()
	{
		if (!instance_)
		{
			instance_ = new PlayerDataManager();
		}
		return *instance_;
	}

	static void Destroy()
	{
		delete instance_;
		instance_ = nullptr;
	}

	PlayerTOTPData* GetPlayer(int playerid)
	{
		if (playerid < 0 || playerid >= MAX_PLAYERS)
		{
			return nullptr;
		}
		return &players_[playerid];
	}

	void ResetPlayer(int playerid)
	{
		if (playerid >= 0 && playerid < MAX_PLAYERS)
		{
			players_[playerid].reset();
		}
	}

	void ResetAll()
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			players_[i].reset();
		}
	}
};
