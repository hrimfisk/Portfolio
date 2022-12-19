// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameLibraryThread.h: Wrapper class to abstract the threading process.
=============================================================================*/

#include "SaveGameLibraryThread.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "../../Public/SaveGameObject.h"
#include "../InternalSystem/SaveGameEvents.h"
#include "../Utils/LogDeclarations.h"
#include "../Structs/SaveGameData.h"

SaveGameLibraryThread::SaveGameLibraryThread(USaveGameObject* InSaveGameObject, const FString& InFilename, const FString& InSaveSlotName, const int& InUserIndex)
    : SaveGameObject{ InSaveGameObject }
    , Filename{ InFilename }
    , SaveSlotName{ InSaveSlotName }
    , UserIndex{ InUserIndex }

{
    Thread = FRunnableThread::Create(this, TEXT("GameSaver"));
}

SaveGameLibraryThread::~SaveGameLibraryThread()
{
    Thread->Kill();
}

uint32 SaveGameLibraryThread::Run()
{
    if (!SaveGameObject)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Unable to save game on separate thread: no save game detected"));
        return -1; //return error
    }

    if (SaveSlotName.IsEmpty())
    {
        UE_LOG(LogSaveGame, Error, TEXT("Unable to save game: no file name specified in ProjectSettings under Plugins->Save Game Settings"));
        return -1;
    }

    if (UGameplayStatics::SaveGameToSlot(SaveGameObject, Filename, UserIndex))
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully saved game on separate thread %s: calling PostSave on all existing Actors"), *SaveSlotName);
        return 0;
    }
    return -1;
}