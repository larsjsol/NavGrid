#pragma once

#include "Modules/ModuleManager.h"

class NavGridPluginImpl : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();
};