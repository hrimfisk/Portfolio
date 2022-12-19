// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveGameLibrary.h: The module that glues the plugin to the project
=============================================================================*/

#pragma once

#include "Modules/ModuleManager.h"

class FSaveGameLibraryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
