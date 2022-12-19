// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveGameLibrary.h: The module that glues the plugin to the project.
=============================================================================*/

#include "../Public/SaveGameLibraryModule.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "SaveSystemSettings.h"

#define LOCTEXT_NAMESPACE "FSaveGameLibraryModule"

void FSaveGameLibraryModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Save Game System",
			LOCTEXT("RuntimeSettingsName", "Save Game Settings"),
			LOCTEXT("RuntimeSettingsDescription", "Configure settings for saving and loading the game"),
			GetMutableDefault<USaveGameSystem_Settings>());
	}
}

void FSaveGameLibraryModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Save Game System");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSaveGameLibraryModule, SaveGameLibrary)