#pragma once
#pragma warning (disable : 4267 4819 26812) // .size() return size_t , enum 유니코드로 저장
#include "WinObject.h"
#include <vector>
#include "Defines.h"

#define WINDOW_SIZE_WIDTH 256
#define WINDOW_SIZE_HEIGHT 256

#define MAP_SIZE_WH 128

#define GRID_SIZE 8

#define M_PI 3.14159265358979323846



typedef struct st_MapStat
{
	int iType;
	int iRegionIndex;
	bool bIsBuildWall;
	bool bIsMove;
}MAP_STAT;



struct st_Rect
{
	int Left;
	int Top;
	int Right;
	int Bottom;

	st_Pos Center;

	int Width;
	int Height;

	int RLeft;
	int RTop;
	int RRight;
	int RBottom;
	st_Rect() { Left = 0; Top = 0; Right = 0; Bottom = 0; }
	st_Rect(int left, int top, int right, int bottom)
	{
		Left = left, Top = top, Right = right, Bottom = bottom;
		RLeft = left * 8, RTop = top * 8, RRight = right * 8, RBottom = bottom * 8;
		Width = Right - Left;
		Height = Bottom - Top;
		Center.X = (Left + Right) / 2;
		Center.Y = (Top + Bottom) / 2;
	}
	bool IsInRect(st_Pos pos)
	{
		if (Left <= pos.X && pos.X <= Right &&
			Top <= pos.Y && pos.Y <= Bottom)
			return true;
		
		return false;
	}
};

struct st_Part
{
	__int64 Key;
	int type;
	st_Pos pos;
	st_Pos RenderPos;
	int Dir;

	void SetRenderPos(int RenderSize)
	{
		RenderPos.X = pos.X * RenderSize;
		RenderPos.Y = pos.Y * RenderSize;
	}
};

typedef struct st_TimerWheel
{
	int TotalTimer;
	int RemaindTime;
	int Round;
	int Index;
	unsigned int PushTime;

	__int64 TWheel_ID;
	WinObject* pObj;
	
	int eUpdateType;
	int iSkillIndex;

	LARGE_INTEGER DebugTime;
	bool TimeCheck();
	bool Update(LARGE_INTEGER freq);
}TWHEEL;

typedef struct st_RegionIndex
{
	int index;
	int Weight;
	int BaseWeight;
	st_RegionIndex(){
		index = 0; Weight = 0; BaseWeight = 0;
	}
	st_RegionIndex(int _i, int _d) {
		index = _i; Weight = _d; BaseWeight = _d;
	};

	// 내림차순을 위한 연산자 재정의
	bool operator < (const st_RegionIndex& a) const { return this->Weight > a.Weight; }
}REGION_WEIGHT;

struct VecF {
	float X; float Y; VecF() {X = 0, Y = 0;};VecF(float _X, float _Y) : X(_X), Y(_Y) {};
};

typedef struct SHAPE
{
	VecF Center;
	VecF U, V;

	int Width, HalfWidth;
	int Height, HalfHeight;

	int Yaw;

	float MinX, MinY;
	float MaxX, MaxY;

	SHAPE(int _X, int _Y, int _Width, int _Height, int _Yaw);
	SHAPE() {};

	std::vector<int> contains(const std::vector<st_Pos>& points);
	bool InArea(st_Pos pos);
}GRENADE;