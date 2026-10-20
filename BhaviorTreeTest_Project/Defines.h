#pragma once

#include <math.h>

enum eSTAT_TYPE
{
	None,
	Wall,
	Window,
	Door,
};

enum eDirection
{
	North,
	South,
	West,
	East
};

enum class eTeam
{
	None,
	Red,
	Blue,
};

enum class eState
{
	None,
	Spawn,
	Idle,
	Combat,
	Occupy,
	Dead,
	End,
};

enum class eCBlackBoardStatus
{
	None,
	Combat,
	Occupy,
	RunAway,
};

enum eTUpdateType
{
	Object,
	CoolTime,
	DuringTime,
	Event,
	End,
};

enum eBTresult
{
	Failed,
	Success,
	Running,
};

enum class eAstarTargetType
{
	None,
	Region,
	InRegionRandomPos,
	Enemy,
};

typedef struct st_Pos
{
	int X;
	int Y;
	st_Pos() { X = 0; Y = 0; }
	st_Pos(int x, int y) { X = x; Y = y; }

	bool operator ==(const st_Pos& pos) const {
		if (this->X == pos.X && this->Y == pos.Y) return true;
		return false;
	}

	int Length(const st_Pos& pos) const
	{
		int dX = X - pos.X;
		int dY = Y - pos.Y;
		return static_cast<int>(sqrt(dX * dX + dY * dY));
	}

	int LengthLength(const st_Pos& pos) const
	{
		int dX = X - pos.X;
		int dY = Y - pos.Y;
		return (dX * dX + dY * dY);
	}

}POS;

struct st_PosInfo
{
	int iType;
	st_Pos pos;
};
