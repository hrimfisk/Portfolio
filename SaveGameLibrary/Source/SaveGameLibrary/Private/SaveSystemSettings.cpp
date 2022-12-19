// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveSystemSettings.h: Class containing settings found in Project Settings 
    under Plugins. These settings are read when the engine first starts and when
    SaveGame or LoadGame is called if the SaveGameObject is not set.
=============================================================================*/

#include "SaveSystemSettings.h"
#include "GeneralProjectSettings.h"
#include "Utils/LogDeclarations.h"

USaveGameSystem_Settings::USaveGameSystem_Settings(const FObjectInitializer& Obj)
    : Super{ Obj }
    , SaveGameObjectClass{ USaveGameObject::StaticClass() }
    , bVerboseLogging{ true }
    , bMultithreadedSaving{ true }
    , bMultithreadedEvents{ false }
    , bSingleFileSaving{ true }
    , bAutomaticActorSaving{ false }
{
    if (const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>())
    {
        SaveFileName = ProjectSettings->ProjectName;
    }
    UE_LOG(LogSaveGame, Display, TEXT("SaveGameSystem Settings attached to Project Settings"));
}