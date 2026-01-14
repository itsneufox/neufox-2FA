#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <chrono>
#include <string>

constexpr size_t TOTP_SECRET_LENGTH_SAMP = 16;
constexpr int MAX_PLAYERS = 1000;

struct PlayerTOTPData
{
	bool enabled;
	bool verified;
	std::string secret;
	int failedAttempts;
	std::chrono::steady_clock::time_point lastAttempt;

	PlayerTOTPData()
		: enabled(false)
		, verified(false)
		, failedAttempts(0)
		, lastAttempt(std::chrono::steady_clock::time_point::min())
	{
	}

	void reset()
	{
		enabled = false;
		verified = false;
		secret.clear();
		failedAttempts = 0;
		lastAttempt = std::chrono::steady_clock::time_point::min();
	}

	bool hasSecret() const
	{
		return !secret.empty();
	}

	void setSecret(const char* newSecret)
	{
		if (newSecret)
		{
			secret = newSecret;
			if (secret.length() > TOTP_SECRET_LENGTH_SAMP)
				secret.resize(TOTP_SECRET_LENGTH_SAMP);
		}
		else
		{
			secret.clear();
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
