// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveSystemSettings.h: Class containing settings found in Project Settings under Plugins.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "../Public/SaveGameObject.h"
#include "SaveSystemSettings.generated.h"

UCLASS(config = SaveGameSettings)
class USaveGameSystem_Settings : public UObject
{
	GENERATED_BODY()

public:
	USaveGameSystem_Settings(const FObjectInitializer& Obj);

	/* The storage class that will contain all data for the Save Game System.
	The SaveGameObject class is provided by the SaveGameSystem plugin, but that can be subclassed to add specific data or functionality
	*/
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	TSubclassOf<USaveGameObject> SaveGameObjectClass;

	/* The name of the save file. If SingleFileSaving is false, this is ignored because each file is named according to the SaveSlotName */
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	FString SaveFileName;

	/* Enables all logging. If disabled, only the following events will be logged:
	New Game
	Load Game
	Save Game
	Delete Game
	Settings
	Errors
	*/
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	bool bVerboseLogging;

	UPROPERTY(Config, EditAnywhere, Category = Settings)
	bool bMultithreadedSaving;

	UPROPERTY(Config, EditAnywhere, Category = Settings)
	bool bMultithreadedEvents;

	/* If this is true, all save slots will be saved under the file name specified in SaveFileName 
	If this is false, each save slot will be stored in their own file under the name specified in SaveSlotName */
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	bool bSingleFileSaving;

	/* If this is true, all actors that exist in the world will be saved.
	* Otherwise, you have to specify which actors are saved with the StoreActor node.
	* If you are manually storing actors, this is not recommended
	*/
	UPROPERTY(Config, EditAnywhere, Category = Settings)
	bool bAutomaticActorSaving;
};