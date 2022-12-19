// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	TimeKeeper.h: Actor created to maintain time played through Tick.
	This is necessary because we need something that will Tick but not get
	garbage collected to keep track of how much time has been played.
=============================================================================*/

#include "TimeKeeper.h"
#include "../Structs/SaveGameData.h"
#include "Kismet/GameplayStatics.h"
#include "../Utils/LogDeclarations.h"

// Sets default values
ATimeKeeper::ATimeKeeper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = 1.f;
	//SaveData = nullptr;
	bIsActive = false;
	SaveData = nullptr;
	bIsVerboseLoggingEnabled = true;
}

// Called when the game starts or when spawned
void ATimeKeeper::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATimeKeeper::Init(FSavedData* InSaveData, bool bInIsVerboseLoggingEnabled)
{
	SaveData = InSaveData;
	bIsActive = true;
	bIsVerboseLoggingEnabled = bInIsVerboseLoggingEnabled;
	if (bIsVerboseLoggingEnabled)
	{
		UE_LOG(LogSaveGame, Display, TEXT("TimeKeeper is now active"));
	}
}

void ATimeKeeper::SetIsActive(bool bInIsActive, bool bInIsVerboseLoggingEnabled)
{
	if (bIsVerboseLoggingEnabled)
	{
		if (bInIsActive)
		{
			UE_LOG(LogSaveGame, Display, TEXT("TimeKeeper is now active"));
		}
		else
		{
			UE_LOG(LogSaveGame, Display, TEXT("TimeKeeper is now inactive"));
		}
	}
	bIsActive = bInIsActive;
}

// Called every frame
void ATimeKeeper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsActive)
	{
		++SaveData->SecondsPlayed;
		if (bIsVerboseLoggingEnabled)
		{
			UE_LOG(LogSaveGame, Display, TEXT("Timekeeper: %d seconds have been played"), SaveData->SecondsPlayed);
		}
	}
}
