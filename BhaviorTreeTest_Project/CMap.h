#pragma once

#include "WinObject.h"
#include "CRegion.h"
#include "CEntity.h"
#include "CAstar.h"
#include <vector>
#include <unordered_map>

class CEntity;
class CPartsBase;

class CMap : public WinObject
{
public:
	CMap();
	~CMap();
private:
	int m_iGridRenderSize = 8;
	CAstar m_astar;
	std::vector<CRegion*> m_vecRegion;
	std::unordered_map<__int64, CPartsBase*> m_mapParts;
	MAP_STAT m_vecMapStat[MAP_SIZE_WH][MAP_SIZE_WH];


	st_Pos m_stSpawnPoint[3];

public:
	void Update() override;
	void Render(HDC hdc) override;

	bool FindPath(int sX, int sY, int eX, int eY, std::vector<st_Pos>& vec) {return m_astar.FindPath(sX, sY, eX, eY, vec);};

	bool BresenhamLine(int sX, int sY, int eX, int eY);	// 두 위치가 이어지는가?
	void BresenhamLine(int sX, int sY, int eX, int eY, std::queue<st_PosInfo>& out);
	bool OnCollision(CEntity* pOwner, st_Pos pos, int Damage);
	bool AllRegionOccupy();

	const std::vector<CRegion*>& GetRegionVector() const { return m_vecRegion; }
	CRegion* GetRegion(int index);
	int GetRegionCount() const { return static_cast<int>(m_vecRegion.size()); }
	st_Pos GetSpawnPos(int index);
	int GetNearRegionIndex(st_Pos pos);
	int GetNearRegion(const std::vector<int>& vec, st_Pos pos);
	CPartsBase* GetPart(st_Pos pos);
	int GetRegionIndex(st_Pos pos) { return m_vecMapStat[pos.Y][pos.X].iRegionIndex; }
	eTeam GetRegionOccupyTeam(int regionindex) { return m_vecRegion[regionindex]->GetOccupyTeam(); }
	int GetRegionOppositeCount(int regionindex);

private:
	void RenderGrid(HDC hdc);
	bool FindShape(int sX, int sY, st_Rect& region, std::vector<st_Part>& vecParts);
	bool SearchRegion();

	void AddPart(std::vector<st_Part>& vec, int regionindex);

	void CreateDoor(st_Part& out, eDirection dir, st_Pos pos);
};
