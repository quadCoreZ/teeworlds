/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_STATBOARD_H
#define GAME_CLIENT_COMPONENTS_STATBOARD_H
#include <game/client/component.h>

class CStatboard : public CComponent
{
	void RenderStatboard(float x, float y, float w, int Team, const char *pTitle);

	static void ConKeyStatboard(IConsole::IResult *pResult, void *pUserData);

	bool m_Active;

	class CPlayerStats
	{
		public:
			int m_Kills;
			int m_Deaths;
			CPlayerStats();
			void Reset();
	};
	CPlayerStats m_aPlayerStats[16];


public:
	CStatboard();
	virtual void OnReset();
	virtual void OnConsoleInit();
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnMessage(int MsgType, void *pRawMsg);

	bool Active();
	void ResetPlayerStats(int ClientID); 

};

#endif
