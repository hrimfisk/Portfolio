// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveGameManager.h: The internal class that manages all save slots.
	The blueprint function library uses this as an interface to store and get data
	as well as manage save slots.
=============================================================================*/

#pragma once

#include "SaveGameObject.h"
#include "../Structs/SaveGameData.h"

class ATimeKeeper;
class UWorld;

class SaveGameManager
{
private:
	TArray<FString> SaveSlotNames;
	UWorld* World;
	USaveGameObject* SaveObject;
	ATimeKeeper* TimeKeeper;
    FSaveGameData SaveData;
	FString SaveFileName;

	FSaveSlotInfo CurrentSlotInfo;
	FSavedData CurrentData;
	FString CurrentSlotName;
	bool bHasDataBeenLoaded;
	bool bIsVerboseLoggingEnabled;
	bool bIsUsingSingleSaveFile;
	bool bIsMultithreading;
	bool bIsMultithreadingEvents;

public:
    SaveGameManager();
	
	//////////////////////////////////////////////////////////////////
	//	Utility Functions
	//////////////////////////////////////////////////////////////////

	void SetSaveFileName(const FString& Filename) { SaveFileName = Filename; }
	void SetWorld(UWorld* InWorld) { World = InWorld; }
	void SetIsVerboseLoggingEnabled(bool bNewState) { bIsVerboseLoggingEnabled = bNewState; };
	void SetIsUsingSingleSaveFile(bool bNewState) { bIsUsingSingleSaveFile = bNewState; };
	void SetIsMultithreading(bool bNewState) { bIsMultithreading = bNewState; }
	void SetIsMultithreadingEvents(bool bNewState) { bIsMultithreadingEvents = bNewState; }
	bool DoesSaveSlotExist(const FString& SaveSlotName) const { SaveData.Slots.Contains(SaveSlotName); }

	bool UpdateSaveSlotProjectVersion(const FString& SaveSlotName, const FString& NewVersion);
	bool RenameSlot(const FString& SaveSlotName, const FString& NewSlotName);
	bool NewGame(const FString& SaveSlotName, TSubclassOf<USaveGameObject> SaveGameClass, bool bInIsMultithreading = false, bool bOverwriteExistingSave = false);
	void SaveGame(const FString& SaveSlotName, const int UserIndex, TSubclassOf<USaveGameObject> SaveGameClass, bool bInIsMultithreading = false);
	bool LoadGame(const FString& SaveSlotName, const int UserIndex);
	bool LoadData(const FString& Filename, const int UserIndex);
	bool DeleteGame(const FString& SaveSlotName, const int UserIndex, TSubclassOf<USaveGameObject> SaveGameClass, bool bInIsMultithreading = false);
	void SetActiveSave(const FString& SaveSlotName);
	void SetIsTimeKeeperEnabled(UObject* Context, const bool bNewState);
	TArray<AActor*> SpawnStoredActors(UWorld* InWorld);
	TArray<UObject*> SpawnStoredObjects(UWorld* InWorld);

	//////////////////////////////////////////////////////////////////
	//	Storing Functions
	//////////////////////////////////////////////////////////////////

	void StoreInt(const FString& VariableName, const int Integer);
	void StoreFloat(const FString& VariableName, const float Float);
	void StoreBoolean(const FString& VariableName, const bool Boolean);
	void StoreVector(const FString& VariableName, const FVector& Vector);
	void StoreRotator(const FString& VariableName, const FRotator& Rotator);
	void StoreTransform(const FString& VariableName, const FTransform& Transform);
	void StoreString(const FString& VariableName, const FString& String);
	void StoreText(const FString& VariableName, const FText& Text);
	void StoreName(const FString& VariableName, const FName& Name);
	void StoreClass(const FString& ClassName, TSubclassOf<UObject> Class);
	void StoreActor(const FString& ObjectName, AActor* ActorToStore, ESpawnActorCollisionHandlingMethod SpawnMethod);
	void StoreObject(const FString& ObjectName, UObject* ObjectToStore);
	bool EraseValue(const FString& VariableName);

	//////////////////////////////////////////////////////////////////
	//	Getter Functions for Data
	//////////////////////////////////////////////////////////////////

	int GetInt(const FString& VariableName) const;
	float GetFloat(const FString& VariableName) const;
	bool GetBoolean(const FString& VariableName) const;
	const TSubclassOf<UObject>& GetClass(const FString& ClassName) const;
	const FActorSaveData& GetActorData(const FString& ObjectName) const;
	const FVector& GetVector(const FString& VectorName) const;
	const FTransform& GetTransform(const FString& TransformName) const;
	const FRotator& GetRotator(const FString& RotatorName) const;
	const FString& GetString(const FString& StringLabel) const;
	const FText& GetText(const FString& TextLabel) const;
	const FName& GetName(const FString& NameLabel) const;
	UObject* SpawnObject(const FString& ObjectName, UObject* Outer);
	AActor* SpawnActor(const FString& ObjectName);

	//////////////////////////////////////////////////////////////////
	//	Utility Getter Functions
	//////////////////////////////////////////////////////////////////

	const FString& GetSaveSlotProjectVersion(const FString& SaveSlotName) const;
	const FSaveSlotInfo& GetSaveSlotInfo(const FString& SaveSlotName);
	const TArray<FString>& GetSaveSlotNames();
	const FString& GetCurrentSlotName() const;
	const TArray<FString>& GetActorNames() const;
	const FString& GetDateCreated() const;
	int GetSecondsPlayed() const;
	const FSaveSlotInfo& GetCurrentSlotInfo() const { return CurrentSlotInfo; }
	const FString& GetCurrentSaveSlot() const { return CurrentSlotName; }
	const FString& GetTimePlayed(const FString& SaveSlotName);

private:
	bool GetIsSimulatingPhysics(AActor* Actor) const;
	void AppendTime(FSaveSlotInfo& Slot, int InTime, const FString& TimeAboveOneStrig, const FString& TimeEqualsOneString);
	bool FindAndRenameSlot(const FString& SaveSlotName, const FString& NewSlotName);
	void SpawnTimeKeeper();
	void SpawnActors(UWorld* InWorld, const TArray<FActorSaveData>& ActorData, TArray<AActor*>& SpawnedActors);
	void SpawnObjects(TArray<UObject*>& SpawnedObjects);
	/* Appends the SaveSlotInfo.TimePlayed string to add a plural or singular measure of time */
	void SetTimeAndDateCreated(FSaveSlotInfo& Slot);
};