#pragma once

#include "ModuleManager.h"

class NavGridPluginImpl : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();
};