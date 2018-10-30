/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>

#include <generated/client_data.h>
#include <generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/localization.h>
#include <game/client/render.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/motd.h>
#include <game/client/components/skins.h>

#include "menus.h"
#include "scoreboard.h"
#include "statboard.h"

#include "gameskins.h"
#include <game/client/teesurf.h>

CStatboard::CStatboard()
{
	OnReset();
}

void CStatboard::ConKeyStatboard(IConsole::IResult *pResult, void *pUserData)
{
	((CStatboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CStatboard::OnReset()
{
	m_Active = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
		m_aPlayerStats[i].Reset();
}

void CStatboard::OnRelease()
{
	m_Active = false;
}

void CStatboard::OnConsoleInit()
{
	Console()->Register("+statboard", "", CFGFLAG_CLIENT, ConKeyStatboard, this, "Show statboard");
}

void CStatboard::RenderStatboard(float x, float y, float w, int Team, const char *pTitle)
{
	if(Team == TEAM_SPECTATORS)
		return;

	bool TeamColors = g_Config.m_ClScoreboardOption & TS_SCORE_TEAMCOLORS;
	bool TeamSize = g_Config.m_ClScoreboardOption & TS_SCORE_TEAMSIZE;

	float Colored = 52.0f;
	float Infos = 28.0f;
	float h = 760.0f;
	CUIRect Rect = {x, y, w, h};

	// header background
	CUIRect HeaderRect = {x, y, w, Colored};
	Graphics()->BlendNormal();
	if(TeamColors && m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
		RenderTools()->DrawUIRect(&HeaderRect, vec4(Team ? 0.0f : 1.0f, 0.0f, Team ? 1.0f : 0.0f, 0.35f), CUI::CORNER_T, 17.0f);
	else
		RenderTools()->DrawUIRect(&HeaderRect, vec4(0.0f, 0.0f, 0.0f, 0.6f), CUI::CORNER_T, 17.0f);
	
	// infos background
	CUIRect InfosRect = {x, y + Colored, w, Infos};
	RenderTools()->DrawUIRect(&InfosRect, vec4(0.0f, 0.0f, 0.0f, 0.55f), 0, 0);

	// generell background
	CUIRect BGRect = {x, y + Colored + Infos, w, h - (Colored + Infos)};
	RenderTools()->DrawUIRect(&BGRect, vec4(0.0f, 0.0f, 0.0f, 0.5f), CUI::CORNER_B, 17.0f);

	// render title
	float TitleFontsize = 40.0f;
	if(!pTitle)
	{
		if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			pTitle = Localize("Game over");
		else if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_ROUNDOVER)
			pTitle = Localize("Round over");
		else
			pTitle = Localize("Statboard");
	}
	// team size option
	char aTitle[100];
	str_format(aTitle, sizeof(aTitle), "%s (%d)", pTitle, m_pClient->m_GameInfo.m_aTeamSize[Team]);
	TextRender()->Text(0, x+20.0f, y, TitleFontsize, TeamSize ? aTitle : pTitle, -1);

	char aBuf[128] = {0};
	if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
	{
		int Score = Team == TEAM_RED ? m_pClient->m_Snap.m_pGameDataTeam->m_TeamscoreRed : m_pClient->m_Snap.m_pGameDataTeam->m_TeamscoreBlue;
		str_format(aBuf, sizeof(aBuf), "%d", Score);
	}
	else
	{
		if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_SpectatorID >= 0 && m_pClient->m_Snap.m_SpecInfo.m_SpectatorID < MAX_CLIENTS &&
			m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID])
		{
			int Score = m_pClient->m_Snap.m_paPlayerInfos[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID]->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
		else if(m_pClient->m_Snap.m_pLocalInfo)
		{
			int Score = m_pClient->m_Snap.m_pLocalInfo->m_Score;
			str_format(aBuf, sizeof(aBuf), "%d", Score);
		}
	}
	float tw = TextRender()->TextWidth(0, TitleFontsize, aBuf, -1);
	TextRender()->Text(0, x+w-tw-20.0f, y, TitleFontsize, aBuf, -1);

	// calculate measurements
	x += 10.0f;
	float LineHeight = 60.0f;
	float TeeSizeMod = 0.8f;
	float Spacing = 16.0f;
	if(m_pClient->m_GameInfo.m_aTeamSize[Team] > 12)
	{
		LineHeight = 40.0f;
		TeeSizeMod = 0.8f;
		Spacing = 0.0f;
	}
	else if(m_pClient->m_GameInfo.m_aTeamSize[Team] > 8)
	{
		LineHeight = 50.0f;
		TeeSizeMod = 0.9f;
		Spacing = 8.0f;
	}

	float ScoreOffset = x+10.0f, ScoreLength = 60.0f;
	float TeeOffset = ScoreOffset+ScoreLength, TeeLength = 60*TeeSizeMod;
	float NameOffset = TeeOffset+TeeLength, NameLength = 300.0f-TeeLength;
	float KillsOffset = x+370.0f, KillsLength = 60.0f;
	float DeathsOffset = KillsOffset+KillsLength*2.0f, DeathsLength = 60.0f;
	float RatioOffset = DeathsOffset+DeathsLength*1.5f, RatioLength = 60.0f;
	float NetOffset = RatioOffset+RatioLength*1.5f, NetLength = 60.0f;

	// render headlines
	y += 50.0f;
	float HeadlineFontsize = 22.0f;
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 0.7f);
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Score"), -1);
	TextRender()->Text(0, ScoreOffset+ScoreLength-tw, y, HeadlineFontsize, Localize("Score"), -1);

	TextRender()->Text(0, NameOffset, y, HeadlineFontsize, Localize("Clan/Name"), -1);

	// kills
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Kills"), -1);
	TextRender()->Text(0, KillsOffset+KillsLength-tw, y, HeadlineFontsize, Localize("Kills"), -1);

	// deaths
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Deaths"), -1);
	TextRender()->Text(0, DeathsOffset+DeathsLength-tw, y, HeadlineFontsize, Localize("Deaths"), -1);

	// ratio
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Ratio"), -1);
	TextRender()->Text(0, RatioOffset+RatioLength-tw, y, HeadlineFontsize, Localize("Ratio"), -1);

	// net
	tw = TextRender()->TextWidth(0, HeadlineFontsize, Localize("Net"), -1);
	TextRender()->Text(0, NetOffset+NetLength-tw, y, HeadlineFontsize, Localize("Net"), -1);


	// render player entries
	y += HeadlineFontsize*2.0f;
	float FontSize = 24.0f;
	CTextCursor Cursor;

	for(int RenderDead = 0; RenderDead < 2; ++RenderDead)
	{
		float ColorAlpha = RenderDead ? 0.5f : 1.0f;
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, ColorAlpha);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			// make sure that we render the correct team
			const CGameClient::CPlayerInfoItem *pInfo = &m_pClient->m_Snap.m_aInfoByScore[i];
			if(!pInfo->m_pPlayerInfo || m_pClient->m_aClients[pInfo->m_ClientID].m_Team != Team || (!RenderDead && (pInfo->m_pPlayerInfo->m_PlayerFlags&PLAYERFLAG_DEAD)) ||
				(RenderDead && !(pInfo->m_pPlayerInfo->m_PlayerFlags&PLAYERFLAG_DEAD)))
				continue;

			// background so it's easy to find the local player or the followed one in spectator mode
			if(m_pClient->m_LocalClientID == pInfo->m_ClientID || (m_pClient->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientID == m_pClient->m_Snap.m_SpecInfo.m_SpectatorID))
			{
				Rect.x = x-10.0f;
				Rect.y = y;
				Rect.w = w;
				Rect.h = LineHeight;
				RenderTools()->DrawRoundRect(&Rect, vec4(1.0f, 1.0f, 1.0f, 0.25f*ColorAlpha), 0);
			}

			// score
			str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_pPlayerInfo->m_Score, -999, 999));
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->SetCursor(&Cursor, ScoreOffset+ScoreLength-tw, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = ScoreLength;
			TextRender()->TextEx(&Cursor, aBuf, -1);

			// flag
			if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_FLAGS && m_pClient->m_Snap.m_pGameDataFlag &&
				(m_pClient->m_Snap.m_pGameDataFlag->m_FlagCarrierRed == pInfo->m_ClientID ||
				m_pClient->m_Snap.m_pGameDataFlag->m_FlagCarrierBlue == pInfo->m_ClientID))
			{
				Graphics()->BlendNormal();
				if(!m_pClient->m_pGameSkins->Num())
					Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
				else
					Graphics()->TextureSet(m_pClient->m_pGameSkins->Get(m_pClient->m_pGameSkins->Find(g_Config.m_ClGameSkin))->m_OrgTexture);
				Graphics()->QuadsBegin();

				RenderTools()->SelectSprite(m_pClient->m_aClients[pInfo->m_ClientID].m_Team==TEAM_RED ? SPRITE_FLAG_BLUE : SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);

				float Size = LineHeight;
				IGraphics::CQuadItem QuadItem(TeeOffset+0.0f, y-5.0f-Spacing/2.0f, Size/2.0f, Size);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}

			// avatar
			if(RenderDead)
			{
				Graphics()->BlendNormal();
				Graphics()->TextureSet(g_pData->m_aImages[IMAGE_DEADTEE].m_Id);
				Graphics()->QuadsBegin();
				if(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)
				{
					vec4 Color = m_pClient->m_pSkins->GetColorV4(m_pClient->m_pSkins->GetTeamColor(true, 0, m_pClient->m_aClients[pInfo->m_ClientID].m_Team, SKINPART_BODY), false);
					Graphics()->SetColor(Color.r, Color.g, Color.b, Color.a);
				}
				IGraphics::CQuadItem QuadItem(TeeOffset, y-5.0f-Spacing/2.0f, 64*TeeSizeMod, 64*TeeSizeMod);
				Graphics()->QuadsDrawTL(&QuadItem, 1);
				Graphics()->QuadsEnd();
			}
			else
			{
				CTeeRenderInfo TeeInfo = m_pClient->m_aClients[pInfo->m_ClientID].m_RenderInfo;
				TeeInfo.m_Size *= TeeSizeMod;
				RenderTools()->RenderTee(CAnimState::GetIdle(), &TeeInfo, EMOTE_NORMAL, vec2(1.0f, 0.0f), vec2(TeeOffset+TeeLength/2, y+LineHeight/2));
			}

			// name
			// todo: improve visual player ready state
			if(!(pInfo->m_pPlayerInfo->m_PlayerFlags&PLAYERFLAG_READY))
				TextRender()->TextColor(1.0f, 0.5f, 0.5f, ColorAlpha);
			else if(RenderDead && pInfo->m_pPlayerInfo->m_PlayerFlags&PLAYERFLAG_WATCHING)
				TextRender()->TextColor(1.0f, 1.0f, 0.0f, ColorAlpha);
			TextRender()->SetCursor(&Cursor, NameOffset, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = NameLength;
			char aBuf[64];
			if(g_Config.m_ClShowsocial)
				str_format(aBuf, sizeof(aBuf), "%s: %s", m_pClient->m_aClients[pInfo->m_ClientID].m_aClan, m_pClient->m_aClients[pInfo->m_ClientID].m_aName);
			else
				str_format(aBuf, sizeof(aBuf), "%2d", pInfo->m_ClientID);
			TextRender()->TextEx(&Cursor, aBuf, -1);
			TextRender()->TextColor(1.0f, 1.0f, 1.0f, ColorAlpha);
			
			// score
			str_format(aBuf, sizeof(aBuf), "%d", clamp(pInfo->m_pPlayerInfo->m_Score, -999, 999));
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->SetCursor(&Cursor, ScoreOffset+ScoreLength-tw, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = ScoreLength;
			TextRender()->TextEx(&Cursor, aBuf, -1);

			// kills
			str_format(aBuf, sizeof(aBuf), "%d", clamp(m_aPlayerStats[pInfo->m_ClientID].m_Kills, 0, 999));
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->SetCursor(&Cursor, KillsOffset+KillsLength/2-tw/2, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = KillsLength;
			TextRender()->TextEx(&Cursor, aBuf, -1);

			// deaths
			str_format(aBuf, sizeof(aBuf), "%d", clamp(m_aPlayerStats[pInfo->m_ClientID].m_Deaths, 0, 999));
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->SetCursor(&Cursor, DeathsOffset+DeathsLength/2-tw/2, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = DeathsLength;
			TextRender()->TextEx(&Cursor, aBuf, -1);

			// ratio
			if(m_aPlayerStats[pInfo->m_ClientID].m_Kills != 0 && m_aPlayerStats[pInfo->m_ClientID].m_Deaths != 0)
				str_format(aBuf, sizeof(aBuf), "%.2f", (float)m_aPlayerStats[pInfo->m_ClientID].m_Kills / (float)m_aPlayerStats[pInfo->m_ClientID].m_Deaths);
			else if(m_aPlayerStats[pInfo->m_ClientID].m_Kills != 0 && m_aPlayerStats[pInfo->m_ClientID].m_Deaths == 0)
				str_format(aBuf, sizeof(aBuf), "%d.00",clamp(m_aPlayerStats[pInfo->m_ClientID].m_Kills, 0, 99));
			else
				str_format(aBuf, sizeof(aBuf), "0.00");
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->SetCursor(&Cursor, RatioOffset+RatioLength/2-tw/2, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = RatioLength;
			TextRender()->TextEx(&Cursor, aBuf, -1);

			// net
			if(m_aPlayerStats[pInfo->m_ClientID].m_Kills == m_aPlayerStats[pInfo->m_ClientID].m_Deaths)
				str_format(aBuf, sizeof(aBuf), "0");
			else
				str_format(aBuf, sizeof(aBuf), "%+d", m_aPlayerStats[pInfo->m_ClientID].m_Kills - m_aPlayerStats[pInfo->m_ClientID].m_Deaths);
			tw = TextRender()->TextWidth(0, FontSize, aBuf, -1);
			TextRender()->SetCursor(&Cursor, NetOffset+NetLength/2-tw/2, y+Spacing, FontSize, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = NetLength;
			TextRender()->TextEx(&Cursor, aBuf, -1);

			TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
			y += LineHeight+Spacing;
		}
	}
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void CStatboard::OnRender()
{
	if(!Active())
		return;

	// if the statboard is active, then we should clear the motd message aswell
	if(m_pClient->m_pMotd->IsActive())
		m_pClient->m_pMotd->Clear();


	float Width = 400*3.0f*Graphics()->ScreenAspect();
	float Height = 400*3.0f;

	Graphics()->MapScreen(0, 0, Width, Height);

	float w = 760.0f;
	float y = 170.0f;

	if(m_pClient->m_Snap.m_pGameData)
	{
		if(!(m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS))
			RenderStatboard(Width/2-w/2, y, w, 0, 0);
		else if(m_pClient->m_Snap.m_pGameDataTeam)
		{
			const char *pRedClanName = m_pClient->m_pScoreboard->GetClanName(TEAM_RED);
			const char *pBlueClanName = m_pClient->m_pScoreboard->GetClanName(TEAM_BLUE);

			if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
			{
				char aText[256];
				str_copy(aText, Localize("Draw!"), sizeof(aText));

				if(m_pClient->m_Snap.m_pGameDataTeam->m_TeamscoreRed > m_pClient->m_Snap.m_pGameDataTeam->m_TeamscoreBlue)
				{
					if(pRedClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pRedClanName);
					else
						str_copy(aText, Localize("Red team wins!"), sizeof(aText));
				}
				else if(m_pClient->m_Snap.m_pGameDataTeam->m_TeamscoreBlue > m_pClient->m_Snap.m_pGameDataTeam->m_TeamscoreRed)
				{
					if(pBlueClanName)
						str_format(aText, sizeof(aText), Localize("%s wins!"), pBlueClanName);
					else
						str_copy(aText, Localize("Blue team wins!"), sizeof(aText));
				}

				float w = TextRender()->TextWidth(0, 86.0f, aText, -1);
				TextRender()->Text(0, Width/2-w/2, 39, 86.0f, aText, -1);
			}
			else if(m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_ROUNDOVER)
			{
				char aText[256];
				str_copy(aText, Localize("Round over!"), sizeof(aText));

				float w = TextRender()->TextWidth(0, 86.0f, aText, -1);
				TextRender()->Text(0, Width/2-w/2, 39, 86.0f, aText, -1);
			}

			RenderStatboard(Width/2-w-5.0f, y, w, TEAM_RED, pRedClanName ? pRedClanName : Localize("Red team"));
			RenderStatboard(Width/2+5.0f, y, w, TEAM_BLUE, pBlueClanName ? pBlueClanName : Localize("Blue team"));
		}
	}

	m_pClient->m_pScoreboard->RenderRecordingNotification((Width/7)*6);
}

bool CStatboard::Active()
{
	// if we activly wanna look on the statboard
	if(m_Active && !m_pClient->m_pMenus->IsActive())
		return true;

	// skip if motd is present
	if(m_pClient->m_pMotd->IsActive())
		return false;

	return false;
}

void CStatboard::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		m_aPlayerStats[pMsg->m_Victim].m_Deaths++;
		if(pMsg->m_Victim != pMsg->m_Killer)
			m_aPlayerStats[pMsg->m_Killer].m_Kills++;
	}
	else if(MsgType == NETMSGTYPE_SV_CLIENTDROP && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_ClientDrop *pMsg = (CNetMsg_Sv_ClientDrop *)pRawMsg;
		if(m_pClient->m_LocalClientID == pMsg->m_ClientID || !m_pClient->m_aClients[pMsg->m_ClientID].m_Active)
		{
			if(g_Config.m_Debug)
				Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "client", "invalid clientdrop");
			return;
		}
		ResetPlayerStats(pMsg->m_ClientID);
	}
}

void CStatboard::ResetPlayerStats(int ClientID)
{
	m_aPlayerStats[ClientID].Reset();
}

CStatboard::CPlayerStats::CPlayerStats()
{
	m_Kills = 0;
	m_Deaths = 0;
} 
void CStatboard::CPlayerStats::Reset()
{
	m_Kills = 0;
	m_Deaths = 0;
}