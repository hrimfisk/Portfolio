// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	ActorSaveData.h: The data that gets saved when StoreActor is used.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActorSaveData.generated.h"

USTRUCT(Blueprintable)
struct FActorSaveData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	TSubclassOf<AActor> Class = nullptr; //8 bytes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	FTransform Transform{}; //48 bytes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	ESpawnActorCollisionHandlingMethod SpawnMethod = ESpawnActorCollisionHandlingMethod::Undefined;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save Game System")
	bool bIsSimulatingPhysics = false;
};

//transform = 48 bytes
//subclass = 48 + 8 = 56
//spawnMethod = 56 + 1 = 57
//physics = 57 + 1 = 58
//6 bytes left until size of FActorSaveData increases