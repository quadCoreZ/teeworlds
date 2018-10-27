/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_GAMESKINS_H
#define GAME_CLIENT_COMPONENTS_GAMESKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

class CGameSkins : public CComponent
{
public:
	// do this better and nicer
	struct CGameSkin
	{

		bool operator<(const CGameSkin &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();

	int Num();
	const CGameSkin *Get(int Index);
	int Find(const char *pName);

private:
	sorted_array<CGameSkin> m_aGameSkins;

	static int GameSkinScan(const char *pName, int IsDir, int DirType, void *pUser);
};
#endif
