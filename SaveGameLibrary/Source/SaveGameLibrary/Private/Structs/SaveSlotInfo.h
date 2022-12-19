// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveSlotInfo.h: Time and date information for save slots.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveSlotInfo.generated.h"

USTRUCT(Blueprintable)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	FString DateCreated;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	FString TimeCreated;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	FString TimePlayed;
};
