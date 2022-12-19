// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveGameData.h: The data that save game slots manage.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "SavedData.h"
#include "SaveSlotInfo.h"

#include "SaveGameData.generated.h"

/**
 * 
 */
USTRUCT()
struct FSaveGameData
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FString, FSaveSlotInfo> Slots;
	UPROPERTY()
	TMap<FString, FSavedData> Data;

	FSaveGameData();
};
