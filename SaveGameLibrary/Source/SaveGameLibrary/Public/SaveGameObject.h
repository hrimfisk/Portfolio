// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveGameObject.h: Subclass of USaveGame for saving data to a slot.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "../Private/Structs/SaveGameData.h"

#include "SaveGameObject.generated.h"


/**
 * 
 */
UCLASS()
class USaveGameObject : public USaveGame
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TMap<FString, FSaveSlotInfo> Slots;
	UPROPERTY()
	TMap<FString, FSavedData> Data;

public:
	USaveGameObject();

	void SetData(const FSaveGameData& InData);
	const TMap<FString, FSaveSlotInfo> GetSlots() const { return Slots; }
	const TMap<FString, FSavedData> GetData() const { return Data; }
};