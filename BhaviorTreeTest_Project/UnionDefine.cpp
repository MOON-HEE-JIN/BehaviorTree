#include "UnionDefine.h"

SHAPE::SHAPE(int _X, int _Y, int _Width, int _Height, int _Yaw)
{
	Center = VecF(_X, _Y);
	Width = _Width, HalfWidth = _Width * 0.5f;
	Height = _Height, HalfHeight = _Height * 0.5f;
	Yaw = _Yaw;

	float DirX = static_cast<float>(cos(Yaw * M_PI));
	float DirY = static_cast<float>(sin(Yaw * M_PI));

	U = VecF(DirX, DirY);
	V = VecF(-DirY, DirX);

	float extX = fabsf(U.X * HalfWidth) + fabsf(V.X * HalfHeight);
	float extY = fabsf(U.Y * HalfWidth) + fabsf(V.Y * HalfHeight);

	MinX = Center.X - extX; MaxX = Center.X + extX;
	MinY = Center.Y - extY; MaxY = Center.Y + extY;
}

std::vector<int> SHAPE::contains(const std::vector<st_Pos>& points)
{
	std::vector<int> vecRet;
	int nLoop = points.size();

	for (int i = 0; i < nLoop; i++)
	{
		float projU = (points[i].X - Center.X) * U.X + (points[i].Y - Center.Y) * U.Y;
		float projV = (points[i].X - Center.X) * V.X + (points[i].Y - Center.Y) * V.Y;

		if (fabsf(projU) <= HalfWidth && fabsf(projV) <= HalfHeight)
			vecRet.push_back(i);
	}

	return vecRet;
}

bool SHAPE::InArea(st_Pos pos)
{
	float projU = (pos.X - Center.X) * U.X + (pos.Y - Center.Y) * U.Y;
	float projV = (pos.X - Center.X) * V.X + (pos.Y - Center.Y) * V.Y;

	if (fabsf(projU) <= HalfWidth && fabsf(projV) <= HalfHeight)
		return true;

	return false;
}
