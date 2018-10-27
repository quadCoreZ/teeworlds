#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <engine/storage.h>

#include "gameskins.h"

int CGameSkins::GameSkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CGameSkins *pSelf = (CGameSkins *)pUser;
	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
		return 0;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "gameskins/%s", pName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load gameskin from %s", pName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}

	CGameSkin GameSkin;
	GameSkin.m_OrgTexture = pSelf->Graphics()->LoadTextureRaw(1024, 512, Info.m_Format, Info.m_pData, Info.m_Format, 0);

	mem_free(Info.m_pData);

	// set gameskin data
	str_copy(GameSkin.m_aName, pName, min((int)sizeof(GameSkin.m_aName),l-3));
	str_format(aBuf, sizeof(aBuf), "load gameskin %s", GameSkin.m_aName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	pSelf->m_aGameSkins.add(GameSkin);

	return 0;
}


void CGameSkins::OnInit()
{
	// load gameskins
	m_aGameSkins.clear();
	Storage()->ListDirectory(IStorage::TYPE_ALL, "gameskins", GameSkinScan, this);
	if(!m_aGameSkins.size())
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "gameclient", "failed to load gameskins. folder='gameskins/'");
}

int CGameSkins::Num()
{
	return m_aGameSkins.size();
}

const CGameSkins::CGameSkin *CGameSkins::Get(int Index)
{
	return &m_aGameSkins[max(0, Index%m_aGameSkins.size())];
}

int CGameSkins::Find(const char *pName)
{
	for(int i = 0; i < m_aGameSkins.size(); i++)
	{
		if(str_comp(m_aGameSkins[i].m_aName, pName) == 0)
			return i;
	}
	return -1;
}