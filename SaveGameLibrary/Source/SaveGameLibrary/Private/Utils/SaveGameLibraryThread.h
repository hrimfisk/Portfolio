// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameLibraryThread.h: Wrapper class to abstract the threading process.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"

#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

class USaveGameObject;

class SaveGameLibraryThread : public FRunnable
{
private:
    FRunnableThread* Thread;
    USaveGameObject* SaveGameObject;
    const FString& Filename;
    const FString& SaveSlotName;
    const int& UserIndex;

public:
    //constructor
    SaveGameLibraryThread(USaveGameObject* InSaveGameObject, const FString& InFilename, const FString& InSaveSlotName, const int& InUserIndex);
    ~SaveGameLibraryThread();
    uint32 Run() override;
};