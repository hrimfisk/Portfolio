// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SavedData.h: The data that gets saved to each slot.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "ActorSaveData.h"

#include "SavedData.generated.h"


USTRUCT(Blueprintable)
struct FSavedData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	float Version;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TArray<FString> ActorNames;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FActorSaveData> Actors;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, TSubclassOf<UObject>> Objects;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, TSubclassOf<UObject>> Classes;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FTransform> Transforms;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FVector> Vectors;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FRotator> Rotators;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FString> Strings;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FText> Texts;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, FName> Names;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, int> Integers;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, float> Floats;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	TMap<FString, bool> Booleans;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	int SecondsPlayed;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Expanded Save Game Library")
	FString ProjectVersion;
	
	FSavedData();
};