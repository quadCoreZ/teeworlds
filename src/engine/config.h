/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include "kernel.h"

class IConfig : public IInterface
{
	MACRO_INTERFACE("config", 0)
public:
	typedef void (*SAVECALLBACKFUNC)(IConfig *pConfig, void *pUserData);

	virtual void Init(int FlagMask) = 0;
	virtual void Reset() = 0;
	virtual void RestoreStrings() = 0;
	virtual void Save(const char *pFilename=0) = 0;
	virtual void SaveTeeSurf() = 0;

	virtual void RegisterCallback(SAVECALLBACKFUNC pfnFunc, void *pUserData) = 0;

	virtual void WriteLine(const char *pLine) = 0;
};

extern IConfig *CreateConfig();

#endif
