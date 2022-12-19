// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	TimeKeeper.h: Actor created to maintain time played through Tick.
	This is necessary because we need something that will Tick but not get
	garbage collected to keep track of how much time has been played.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "../Structs/SaveGameData.h"

#include "TimeKeeper.generated.h"

UCLASS()
class ATimeKeeper: public AActor
{
	GENERATED_BODY()
	
private:
	FSavedData* SaveData;
	bool bIsActive;
	bool bIsVerboseLoggingEnabled;

public:	
	// Sets default values for this actor's properties
	ATimeKeeper();

	void Init(FSavedData* InSaveData, bool bInIsVerboseLoggingEnabled);
	void SetIsActive(bool bInIsActive, bool bInIsVerboseLoggingEnabled);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
