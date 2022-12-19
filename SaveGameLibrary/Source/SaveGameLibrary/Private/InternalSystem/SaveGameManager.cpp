// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameManager.cpp: The internal class that manages all save slots.
    The blueprint function library uses this as an interface to store and get 
    data, as well as manage save slots. This class also creates the SaveGameLibraryThread
    object if multithreading is enabled and creates the TimeKeeper object.
    All library functions go through this interface to handle anything with save
    slots.
=============================================================================*/

#include "SaveGameManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Containers/Array.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/BuildVersion.h"

#include "Utils/SaveGameLibraryThread.h"
#include "Utils/LogDeclarations.h"
#include "Utils/EventThreads.h"
#include "SaveGameEvents.h"
#include "TimeKeeper.h"

using Statics = UGameplayStatics;

//default variables used as return value placeholders because we cant return const reference if the value doesnt exist
//and returning a reference to a temp variable is not recommended
static const FString EmptyString = FString{};
static const FText EmptyText = FText{};
static const FName EmptyName = FName{};
static const FVector EmptyVector = FVector{ 0.f, 0.f, 0.f };
static const FRotator EmptyRotator = FRotator{ 0.f, 0.f, 0.f };
static const FTransform EmptyTransform = FTransform{ EmptyRotator, EmptyVector, EmptyVector };
static const FActorSaveData EmptyActorData = FActorSaveData{};
static const FSaveSlotInfo EmptySlotInfo = FSaveSlotInfo{};
static const FSavedData EmptySaveData = FSavedData{};
static const TSubclassOf<UObject> EmptyClass = nullptr;

#define CHECK_IS_NULL(value, name, message)                                                  \
if (!value)                                                                                  \
{                                                                                            \
    UE_LOG(LogSaveGame, Warning, TEXT(message), *name); \
    return;                                                                                  \
}

#define INTERNAL_GET_VALUE(func, map, type, emptyValue)                                                             \
type SaveGameManager::Get##func(const FString& VariableName) const    \
{                                                                 \
    if (!CurrentData.map.Contains(VariableName))                                                     \
    {                                                                                         \
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to get %s: value doesn't exist"), *VariableName);   \
        return emptyValue;                                                                   \
    }                                                                                         \
    return CurrentData.map[VariableName];                                                            \
}

#define INTERNAL_ERASE_VALUE(map)                                                               \
if (CurrentData.map.Contains(VariableName))                                                          \
{                                                                                            \
    CurrentData.map.Remove(VariableName);                                                            \
    UE_LOG(LogSaveGame, Warning, TEXT("Successfully erased value %s from %s"), *VariableName, #map);               \
    bDidEraseValue = true; \
}                                                                                            \

#define INTERNAL_LOG(output, value)                                            \
if (bIsVerboseLoggingEnabled)                                         \
{                                                                     \
    UE_LOG(LogSaveGame, Display, TEXT(output), *VariableName, value); \
}
#define INTERNAL_STORE(container, func, type, log)                                 \
void SaveGameManager::Store##func(const FString& VariableName, type Value)\
{                                                                         \
    CurrentData.##container.Add(VariableName, Value);                     \
    log;                                                                  \
}

#define INTERNAL_STORE_VALUE(container, func, type, output) INTERNAL_STORE(container, func, type, INTERNAL_LOG(output, Value))
#define INTERNAL_STORE_TEXT_VALUE(container, func, type, output, val) INTERNAL_STORE(container, func, type, INTERNAL_LOG(output, val))

//Generate similar enough functions to reduce line count
//BEGINNING OF GETTERS AND SETTERS
INTERNAL_STORE_VALUE(Integers, Int, const int, "Successfully stored int %s as %d")
INTERNAL_STORE_VALUE(Floats, Float, const float, "Successfully stored float %s as %f")
INTERNAL_STORE_TEXT_VALUE(Booleans, Boolean, const bool, "Successfully stored bool %s as %s", (Value ? "enabled" : "disabled"))
INTERNAL_STORE_TEXT_VALUE(Texts, Text, const FText&, "Successfully stored text %s as %s", *Value.ToString())
INTERNAL_STORE_TEXT_VALUE(Names, Name, const FName&, "Successfully stored name %s as %s", *Value.ToString())
INTERNAL_STORE_TEXT_VALUE(Strings, String, const FString&, "Successfully stored string %s as %s", *Value)

INTERNAL_GET_VALUE(Int, Integers, int, 0)
INTERNAL_GET_VALUE(Float, Floats, float, 0.f)
INTERNAL_GET_VALUE(Boolean, Booleans, bool, false)
INTERNAL_GET_VALUE(Class, Classes, const TSubclassOf<UObject>&, EmptyClass)
INTERNAL_GET_VALUE(ActorData, Actors, const FActorSaveData&, EmptyActorData)
INTERNAL_GET_VALUE(Vector, Vectors, const FVector&, EmptyVector)
INTERNAL_GET_VALUE(Transform, Transforms, const FTransform&, EmptyTransform)
INTERNAL_GET_VALUE(Rotator, Rotators, const FRotator&, EmptyRotator)
INTERNAL_GET_VALUE(String, Strings, const FString&, EmptyString)
INTERNAL_GET_VALUE(Text, Texts, const FText&, EmptyText)
INTERNAL_GET_VALUE(Name, Names, const FName&, EmptyName)
//END OF GETTERS AND SETTERS

SaveGameManager::SaveGameManager()
    : SaveSlotNames{}
    , World{ nullptr }
    , SaveObject{ nullptr }
    , TimeKeeper{ nullptr }
    , SaveData{}
    , SaveFileName{}
    , CurrentSlotName{}
    , bHasDataBeenLoaded{ false }
    , bIsVerboseLoggingEnabled{ true }
    , bIsUsingSingleSaveFile{ true }
    , bIsMultithreading{ true }
    , bIsMultithreadingEvents{ false }
{}

void SaveGameManager::StoreVector(const FString& VariableName, const FVector& Vector)
{
    CurrentData.Vectors.Add(VariableName, Vector);
    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully stored vector %s as { %f, %f, %f }"), *VariableName, Vector.X, Vector.Y, Vector.Z);
    }
}

void SaveGameManager::StoreRotator(const FString& VariableName, const FRotator& Rotator)
{
    CurrentData.Rotators.Add(VariableName, Rotator);
    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully stored rotator %s as { %f, %f, %f }"), *VariableName, Rotator.Pitch, Rotator.Yaw, Rotator.Roll);
    }
}

void SaveGameManager::StoreTransform(const FString& VariableName, const FTransform& Transform)
{
    CurrentData.Transforms.Add(VariableName, Transform);
    const auto& Rotation = Transform.GetRotation();
    const auto& Translation = Transform.GetTranslation();
    const auto& Scale = Transform.GetScale3D();
    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully stored transform %s as { %f, %f, %f, }, { %f, %f, %f }, { %f, %f, %f }"), *VariableName, Rotation.X, Rotation.Y, Rotation.Z, Translation.X, Translation.Y, Translation.Z, Scale.X, Scale.Y, Scale.Z);
    }
};

void SaveGameManager::StoreClass(const FString& ClassName, TSubclassOf<UObject> Class)
{
    CHECK_IS_NULL(Class, ClassName, "Unable to store %s as class: class is null");
    CurrentData.Classes.Add(ClassName, Class);
    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully stored class %s as %s"), *Class->GetName(), *ClassName);
    }
}

void SaveGameManager::StoreActor(const FString& ObjectName, AActor* ActorToStore, ESpawnActorCollisionHandlingMethod SpawnMethod)
{
    CHECK_IS_NULL(ActorToStore, ObjectName, "Unable to store actor %s: Actor is null");
    ISaveGameEvents* Interface = Cast<ISaveGameEvents>(ActorToStore);
    if (Interface)
    {
        Interface->PreSave();
    }

    FString Name = ObjectName;
    if (ObjectName.IsEmpty())
    {
        Name = ActorToStore->GetName();
    }

    //store the physics state of any skeletal or static mesh components
    bool bIsSimulatingPhysics = GetIsSimulatingPhysics(ActorToStore);

    FActorSaveData Data;
    Data.Class = ActorToStore->GetClass();
    Data.Transform = ActorToStore->GetTransform();
    Data.SpawnMethod = SpawnMethod;
    Data.bIsSimulatingPhysics = bIsSimulatingPhysics;
    //if data under the provided name already exists, update it
    if (CurrentData.Actors.Contains(Name))
    {
        auto& ExistingData = CurrentData.Actors[Name];
        ExistingData = MoveTemp(Data);
    }
    //otherwise add it
    else
    {
        CurrentData.Actors.Add(Name, MoveTemp(Data));
        CurrentData.ActorNames.Add(Name);
    }

    if (Interface)
    {
        Interface->PostSave();
    }

    if (bIsVerboseLoggingEnabled)
    {
        const auto& Transform = ActorToStore->GetTransform();
        const auto& Rotation = Transform.GetRotation();
        const auto& Translation = Transform.GetTranslation();
        const auto& Scale = Transform.GetScale3D();
        UE_LOG(LogSaveGame, Display, TEXT("Successfully stored actor %s as %s with transform { %f, %f, %f }, { %f, %f, %f }, { %f, %f, %f }"), *Name, *ActorToStore->GetClass()->GetName(), Rotation.X, Rotation.Y, Rotation.X, Translation.X, Translation.Y, Translation.Z, Scale.X, Scale.Y, Scale.Z);
    }
}

void SaveGameManager::StoreObject(const FString& ObjectName, UObject* ObjectToStore)
{
    CHECK_IS_NULL(ObjectToStore, ObjectName, "Unable to store object %s: Object is null");
    ISaveGameEvents* Interface = Cast<ISaveGameEvents>(ObjectToStore);
    if (Interface)
    {
        Interface->PreSave();
    }

    FString Name = ObjectName;
    if (!ObjectName.IsEmpty())
    {
        Name = ObjectToStore->GetName();
    }

    UClass* ObjectClass = ObjectToStore->GetClass();
    if (ObjectClass)
    {
        CurrentData.Objects.Add(Name, ObjectClass);
    }

    if (Interface)
    {
        Interface->PostSave();
    }

    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully stored object %s as %s"), *Name, *ObjectClass->GetName());
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//                      END OF DATA SETTERS/GETTERS                                //
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

bool SaveGameManager::UpdateSaveSlotProjectVersion(const FString& SaveSlotName, const FString& NewVersion)
{
    if (SaveData.Slots.Contains(SaveSlotName))
    {
        FSavedData& Slot = SaveData.Data[SaveSlotName];
        Slot.ProjectVersion = NewVersion;
        return true;
    }
    return false;
}

bool SaveGameManager::RenameSlot(const FString& SaveSlotName, const FString& NewSlotName)
{
    if (!SaveData.Slots.Contains(SaveSlotName))
    {
        UE_LOG(LogSaveGame, Display, TEXT("Unable to rename save slot %s: slot doesn't exist"), *SaveSlotName);
        return false;
    }

    if (bIsUsingSingleSaveFile)
    {
        if (CurrentSlotName == SaveSlotName)
        {
            CurrentSlotName = NewSlotName;
            return FindAndRenameSlot(SaveSlotName, NewSlotName);
        }
    }

    return FindAndRenameSlot(SaveSlotName, NewSlotName);
}

bool SaveGameManager::NewGame(const FString& SaveSlotName, TSubclassOf<USaveGameObject> SaveGameClass, bool bInIsMultithreading, bool bOverwriteExistingSave)
{
    if (!IsInGameThread())
    {
        UE_LOG(LogSaveGame, Error, TEXT("NewGame cannot be multithreaded"));
        return false;
    }

    TArray<AActor*> ActorsWithInterface;
    Statics::GetAllActorsWithInterface(World, USaveGameEvents::StaticClass(), ActorsWithInterface);
    bIsMultithreading = bInIsMultithreading;
    UE_LOG(LogSaveGame, Display, TEXT("Attempting to create save slot %s"), *SaveSlotName);
    //if we dont want to overwrite and the slot exists, we failed to create a new game
    if (!bOverwriteExistingSave && SaveData.Slots.Contains(SaveSlotName))
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to create new save game %s: slot already exists"), *SaveSlotName);
        return false;
    }

    CurrentSlotName = SaveSlotName;
    CurrentData = FSavedData{};
    FSaveSlotInfo SlotInfo;
    SetTimeAndDateCreated(SlotInfo);
    SaveData.Slots.Add(SaveSlotName, MoveTemp(SlotInfo));
    SaveData.Data.Add(SaveSlotName, MoveTemp(CurrentData));
    FString ProjectVersion = "1.0";
    GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), 
                       TEXT("ProjectVersion"), ProjectVersion, GGameIni);
    CurrentData.ProjectVersion = ProjectVersion;
    UE_LOG(LogSaveGame, Display, TEXT("Successfully created new game %s: calling OnNewSaveCreated on all existing Actors that use the SaveGameEvents interface"), *SaveSlotName);

    if (bIsMultithreadingEvents)
    {
        NewSaveThread Thread(ActorsWithInterface);
    }
    else
    {
        CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(ActorsWithInterface, Execute_NewSaveCreated);
    }

    if (!World)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error creating TimeKeeper: World is not set"));
        return false;
    }
    SpawnTimeKeeper();
    return true;
}

void SaveGameManager::SaveGame(const FString& SaveSlotName, const int UserIndex, TSubclassOf<USaveGameObject> SaveGameClass, bool bInIsMultithreading)
{
    if (!IsInGameThread())
    {
        UE_LOG(LogSaveGame, Error, TEXT("SaveGame cannot be multithreaded"));
        return;
    }

    TArray<AActor*> ActorsWithInterface;
    Statics::GetAllActorsWithInterface(World, USaveGameEvents::StaticClass(), ActorsWithInterface);
    bIsMultithreading = bInIsMultithreading;
    UE_LOG(LogSaveGame, Display, TEXT("Calling PreSave on all existing Actors in the world that use the SaveGameEvents interface"));
    if (bIsMultithreadingEvents)
    {
        PreSaveThread Thread(ActorsWithInterface);
    }
    else 
    {
        CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(ActorsWithInterface, Execute_PreSave);
    }

    FString ProjectVersion = "1.0";
    GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"),
        TEXT("ProjectVersion"), ProjectVersion, GGameIni);
    CurrentData.ProjectVersion = ProjectVersion;
    if (!SaveData.Slots.Contains(SaveSlotName))
    {
        SaveData.Slots.Add(SaveSlotName, CurrentSlotInfo);
        SaveData.Data.Add(SaveSlotName, CurrentData);
    }
    else
    {
        SaveData.Data[SaveSlotName] = CurrentData;
    }

    SaveObject = Cast<USaveGameObject>(Statics::CreateSaveGameObject(SaveGameClass));

    //if not using single save file, save file name is save slot name
    if (!bIsUsingSingleSaveFile)
    {
        SaveFileName = SaveSlotName;
    }

    if (SaveObject)
    {
        SaveObject->SetData(SaveData);
        if (bInIsMultithreading)
        {
            SaveGameLibraryThread(SaveObject, SaveFileName, SaveSlotName, UserIndex);
        }
        else
        {
            if (UGameplayStatics::SaveGameToSlot(SaveObject, SaveFileName, UserIndex))
            {
                UE_LOG(LogSaveGame, Display, TEXT("Successfully saved game %s"), *SaveSlotName);
            }
            else
            {
                UE_LOG(LogSaveGame, Error, TEXT("Error saving game %s: Save Game Object Class or Save File Name are not set in Project Settings under Plugins->Save Game Settings"), *SaveSlotName);
            }
        }
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error saving game %s: Save Game Object could not be created. Save Game Object Class is likely not set in Project Settings under Plugins->Save Game Settings"), *SaveSlotName)
    }

    UE_LOG(LogSaveGame, Display, TEXT("Calling PostSave on all existing Actors in the world that use the SaveGameEvents interface"));
    if (bIsMultithreadingEvents)
    {
        PostSaveThread Thread(ActorsWithInterface);
    }
    else
    {
        CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(ActorsWithInterface, Execute_PostSave)
    }
}

bool SaveGameManager::LoadGame(const FString& SaveSlotName, const int UserIndex)
{
    if (!IsInGameThread())
    {
        UE_LOG(LogSaveGame, Error, TEXT("LoadGame cannot be multithreaded"));
        return false;
    }
    
    TArray<AActor*> ActorsWithInterface;
    Statics::GetAllActorsWithInterface(World, USaveGameEvents::StaticClass(), ActorsWithInterface);
    UE_LOG(LogSaveGame, Display, TEXT("Calling PreLoad on all existing Actors in the world that use the SaveGameEvents interface"))
    if (bIsMultithreadingEvents)
    {
        PreLoadThread Thread(ActorsWithInterface);
    }
    else
    {
        CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(ActorsWithInterface, Execute_PreLoad);
    }

    if (!bIsUsingSingleSaveFile)
    {
        SaveFileName = SaveSlotName;
    }

    if (!bHasDataBeenLoaded)
    {
        LoadData(SaveFileName, UserIndex);
    }

    //todo think about calling SetActiveSave here instead of
    //lines 464 to 472
    if (!SaveData.Slots.Contains(SaveSlotName))
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to load game %s: slot doesn't exist"), *SaveSlotName);
        return false;
    }

    CurrentSlotInfo = SaveData.Slots[SaveSlotName];
    CurrentData = SaveData.Data[SaveSlotName];
    CurrentSlotName = SaveSlotName;

    if (SaveObject && !SaveFileName.IsEmpty())
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully loaded game %s for player %d"), *SaveSlotName, UserIndex);
        UE_LOG(LogSaveGame, Display, TEXT("Calling PostLoad on all existing Actors in the world that use the SaveGameEvents interface"));

        if (bIsMultithreadingEvents)
        {
            PostLoadThread Thread(ActorsWithInterface);
        }
        else
        {
            CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(ActorsWithInterface, Execute_PostLoad);
        }

        if (!World)
        {
            UE_LOG(LogSaveGame, Error, TEXT("Error creating TimeKeeper: World is not set"));
            return false;
        }

        SpawnTimeKeeper();
        return true;
    }
    return false;
}

bool SaveGameManager::LoadData(const FString& Filename, const int UserIndex)
{
    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Attempting to load data for player %d"), UserIndex);
    }

    SaveFileName = Filename;
    if (!Statics::DoesSaveGameExist(SaveFileName, UserIndex))
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to load data : file % s doesn't exist"), *Filename);
    }

    if (bIsMultithreading)
    {
        //if multithreading is enabled, load data on a separate thread
        FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
            [this, UserIndex]()
            {
                SaveObject = Cast<USaveGameObject>(Statics::LoadGameFromSlot(SaveFileName, UserIndex));
            }, TStatId(), NULL, ENamedThreads::GameThread);
        //we have to wait for it to complete so we can use the data
        FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
    }
    else
    {
        SaveObject = Cast<USaveGameObject>(Statics::LoadGameFromSlot(SaveFileName, UserIndex));
    }

    bHasDataBeenLoaded = SaveObject != nullptr;
    if (bHasDataBeenLoaded)
    {
        SaveData.Slots = SaveObject->GetSlots();
        SaveData.Data = SaveObject->GetData();
        if (bIsVerboseLoggingEnabled)
        {
            UE_LOG(LogSaveGame, Display, TEXT("Successfully loaded data for player %d"), UserIndex);
        }
    }
    return bHasDataBeenLoaded;
}

bool SaveGameManager::DeleteGame(const FString& SaveSlotName, const int UserIndex, TSubclassOf<USaveGameObject> SaveGameClass, bool bInIsMultithreading)
{
    if (!bIsUsingSingleSaveFile)
    {
        SaveFileName = SaveSlotName;
    }

    //we have to load the data first to see if the slot is available for deletion
    if (!bHasDataBeenLoaded)
    {
        LoadData(SaveFileName, UserIndex);
    }

    if (SaveData.Slots.Contains(SaveSlotName))
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to delete save %s: slot doesn't exist"), *SaveSlotName);
    }

    //if the active slot is the slot being deleted, reset the data
    if (CurrentSlotName == SaveSlotName)
    {
        CurrentSlotName.Empty();
        CurrentData = FSavedData();
        CurrentSlotInfo = FSaveSlotInfo();
    }
    SaveData.Slots.Remove(SaveSlotName);
    SaveData.Data.Remove(SaveSlotName);
    UE_LOG(LogSaveGame, Display, TEXT("Successfully deleted save slot "), *SaveSlotName);

    SaveObject = Cast<USaveGameObject>(Statics::CreateSaveGameObject(SaveGameClass));
    SaveObject->SetData(SaveData);
    UGameplayStatics::SaveGameToSlot(SaveObject, SaveFileName, UserIndex);
    return true;
}

void SaveGameManager::SetActiveSave(const FString& SaveSlotName)
{
    if (SaveSlotName.IsEmpty())
    {
        if (bIsVerboseLoggingEnabled)
        {
            UE_LOG(LogSaveGame, Display, TEXT("SaveSlotName is empty: active save set to none"));
        }
        CurrentSlotName.Empty();
        CurrentSlotInfo = EmptySlotInfo;
        CurrentData = EmptySaveData;
        return;
    }

    CHECK_IS_NULL(SaveData.Slots.Contains(SaveSlotName), SaveFileName, "Unable to set %s as current save slot: slot doesn't exist");
    CurrentSlotName = SaveSlotName;
    CurrentSlotInfo = SaveData.Slots[SaveSlotName];
    CurrentData = SaveData.Data[SaveSlotName];
    UE_LOG(LogSaveGame, Display, TEXT("Active save slot set to %s"), *SaveSlotName);

    if (!bIsUsingSingleSaveFile)
    {
        SaveFileName = SaveSlotName;
    }
}

void SaveGameManager::SetIsTimeKeeperEnabled(UObject* Context, const bool bNewState)
{
    World = Context->GetWorld();
    SpawnTimeKeeper();
    if (TimeKeeper)
    {
        TimeKeeper->SetIsActive(bNewState, bIsVerboseLoggingEnabled);
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error setting TimeKeeper state: TimeKeeper could not be spawned"));
    }
}

TArray<AActor*> SaveGameManager::SpawnStoredActors(UWorld* InWorld)
{
    TArray<FActorSaveData> ActorData;
    for (const auto& Actor : CurrentData.Actors)
    {
        ActorData.Add(Actor.Value);
    }

    TArray<AActor*> SpawnedActors;
    if (bIsMultithreading)
    {
        //we need a reference to SpawnedActors so we can add to it and return those added values
        //objects can only be spawned on the game thread, so we have to delegate the task
        FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&]()
            {
                SpawnActors(InWorld, ActorData, SpawnedActors);
                CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(SpawnedActors, Execute_PostLoad);
            }, TStatId(), NULL, ENamedThreads::GameThread);

        FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
    }
    else
    {
        SpawnActors(InWorld, ActorData, SpawnedActors);
        CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(SpawnedActors, Execute_PostLoad);
    }

    return MoveTemp(SpawnedActors);
}

TArray<UObject*> SaveGameManager::SpawnStoredObjects(UWorld* InWorld)
{
    TArray<UObject*> SpawnedObjects;

    if (bIsMultithreading)
    {
        //we need a reference to SpawnedObjects so we can add to it and return those added values
        //objects can only be spawned on the game thread, so we have to delegate the task
        FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady(
            [&]()
            {
                SpawnObjects(SpawnedObjects);
            }, TStatId(), NULL, ENamedThreads::GameThread);

        FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
    }
    else
    {
        SpawnObjects(SpawnedObjects);
    }

    return MoveTemp(SpawnedObjects);
}

bool SaveGameManager::EraseValue(const FString& VariableName)
{
    bool bDidEraseValue = false;
    INTERNAL_ERASE_VALUE(Integers);
    INTERNAL_ERASE_VALUE(Floats);
    INTERNAL_ERASE_VALUE(Booleans);
    INTERNAL_ERASE_VALUE(Vectors);
    INTERNAL_ERASE_VALUE(Transforms);
    INTERNAL_ERASE_VALUE(Rotators);
    INTERNAL_ERASE_VALUE(Strings);
    INTERNAL_ERASE_VALUE(Names);
    INTERNAL_ERASE_VALUE(Texts);
    INTERNAL_ERASE_VALUE(Actors);
    INTERNAL_ERASE_VALUE(Classes);
    INTERNAL_ERASE_VALUE(ActorNames);

    if (bIsVerboseLoggingEnabled)
    {
        if (bDidEraseValue)
        {
            UE_LOG(LogSaveGame, Warning, TEXT("Successfully erased value %s"), *VariableName);
        }
        else
        {
            UE_LOG(LogSaveGame, Warning, TEXT("Unable to erase %s: value doesn't exist"), *VariableName);
        }
    }
    return bDidEraseValue;
}

UObject* SaveGameManager::SpawnObject(const FString& ObjectName, UObject* Outer)
{
    if (CurrentData.Objects.Contains(ObjectName))
    {
        if (UObject* Object = NewObject<UObject>(Outer, CurrentData.Objects[ObjectName]))
        {
            if (bIsVerboseLoggingEnabled)
            {
                UE_LOG(LogSaveGame, Display, TEXT("Successfully spawned object %s"), *ObjectName);
            }
            return Object;
        }
        else
        {
            UE_LOG(LogSaveGame, Error, TEXT("Error spawning object %s: cast to UObject not successful"));
        }
    }
    else
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to spawn object %s: data doesn't exist"));
    }
    return nullptr;
}

AActor* SaveGameManager::SpawnActor(const FString& ObjectName)
{
    if (CurrentData.Actors.Contains(ObjectName))
    {
        FActorSaveData& ActorData = CurrentData.Actors[ObjectName];
        if (AActor* Actor = World->SpawnActor<AActor>(ActorData.Class, ActorData.Transform))
        {
            if (bIsVerboseLoggingEnabled)
            {
                UE_LOG(LogSaveGame, Display, TEXT("Successfully spawned actor %s: Calling PostLoad if it implments SaveGameEvents"), *ObjectName);
            }
            /*check for blueprint interface*/
            if (ISaveGameEvents* Interface = Cast<ISaveGameEvents>(Actor))
            {
                Interface->Execute_PostLoad(Actor);
            }
            /*check for C++ interface*/
            else if (Actor->GetClass()->ImplementsInterface(USaveGameEvents::StaticClass()))
            {
                ISaveGameEvents::Execute_PostLoad(Actor);
            }
            return Actor;
        }
    }
    return nullptr;
}

const FString& SaveGameManager::GetSaveSlotProjectVersion(const FString& SaveSlotName) const
{
    if (SaveData.Slots.Contains(SaveSlotName))
    {
        const FSavedData& data = SaveData.Data[SaveSlotName];
        return data.ProjectVersion;
    }
    return EmptyString;
}

const FSaveSlotInfo& SaveGameManager::GetSaveSlotInfo(const FString& SaveSlotName)
{
    if (SaveData.Slots.Contains(SaveSlotName))
    {
        FSaveSlotInfo& Slot = SaveData.Slots[SaveSlotName];
        Slot.TimePlayed = GetTimePlayed(SaveSlotName);
        return Slot;
    }
    return EmptySlotInfo;
}

const TArray<FString>& SaveGameManager::GetSaveSlotNames()
{
    SaveSlotNames.Empty();
    for (const auto& Slot : SaveData.Slots)
    {
        SaveSlotNames.Add(Slot.Key);
    }
    return SaveSlotNames;
}

const FString& SaveGameManager::GetCurrentSlotName() const
{
    return CurrentSlotName;
}

const TArray<FString>& SaveGameManager::GetActorNames() const
{
    return CurrentData.ActorNames;
}

const FString& SaveGameManager::GetDateCreated() const
{
    return CurrentSlotInfo.DateCreated;
}

int SaveGameManager::GetSecondsPlayed() const
{
    return CurrentData.SecondsPlayed;
}

//this funcion is not const because it modifies SaveSlotInfo
const FString& SaveGameManager::GetTimePlayed(const FString& SaveSlotName)
{
    constexpr int SecondsInAYear = 31'557'600;
    constexpr int SecondsInAMonth = 2'592'000;
    constexpr int SecondsInAWeek = 605'800;
    constexpr int SecondsInADay = 86'400;
    constexpr int SecondsInAnHour = 3'600;
    constexpr int SecondsInAMinute = 60;

    //empty the current string for time played so that it doesnt save the old time
    FSaveSlotInfo& Slot = SaveData.Slots[SaveSlotName];
    Slot.TimePlayed.Empty();
    int TimeLeft = SaveData.Data[SaveSlotName].SecondsPlayed;
    int Years = 0;
    int Months = 0;
    int Weeks = 0;
    int Days = 0;
    int Hours = 0;
    int Minutes = 0;

    if (TimeLeft >= SecondsInAYear)
    {
        Years = TimeLeft / 365 / 24 / 60 / 60;
        TimeLeft -= SecondsInAYear * Years;
        AppendTime(Slot, Years, " years, ", " year, ");
    }
    if (TimeLeft >= SecondsInAMonth)
    {
        Months = TimeLeft / 30 / 24 / 60 / 60;
        TimeLeft -= SecondsInAMonth * Months;
        AppendTime(Slot, Months, " months,", " month, ");
    }
    if (TimeLeft >= SecondsInAWeek)
    {
        Weeks = TimeLeft / 7 / 24 / 60 / 60;
        TimeLeft -= SecondsInAWeek * Weeks;
        AppendTime(Slot, Weeks, " weeks,", " week, ");
    }
    if (TimeLeft >= SecondsInADay)
    {
        Days = TimeLeft / 24 / 60 / 60;
        TimeLeft -= SecondsInADay * Days;
        AppendTime(Slot, Days, " days, ", " day, ");
    }
    if (TimeLeft >= SecondsInAnHour)
    {
        Hours = TimeLeft / 60 / 60;
        TimeLeft -= SecondsInAnHour * Hours;
        AppendTime(Slot, Hours, " hours, ", " hour, ");
    }
    if (TimeLeft >= SecondsInAMinute)
    {
        Minutes = TimeLeft / 60;
        TimeLeft -= SecondsInAMinute * Minutes;
        AppendTime(Slot, Minutes, " minutes, ", " minute, ");
    }

    AppendTime(Slot, TimeLeft, " seconds, ", " second, ");
    return Slot.TimePlayed;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//                      END OF PUBLIC FUNCTIONS                                    //
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

bool SaveGameManager::GetIsSimulatingPhysics(AActor* Actor) const
{
    bool bIsSimulatingPhysics = false;
    const TSet<UActorComponent*>& Components = Actor->GetComponents();
    for (const auto& Component : Components)
    {
        if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Component))
        {
            bIsSimulatingPhysics = Mesh->IsSimulatingPhysics();
        }
        else if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
        {
            bIsSimulatingPhysics = StaticMesh->IsSimulatingPhysics();
        }
    }
    return bIsSimulatingPhysics;
}

void SaveGameManager::AppendTime(FSaveSlotInfo& Slot, int InTime, const FString& TimeAboveOneString, const FString& TimeEqualsOneString)
{
    if (InTime > 0)
    {
        if (InTime > 1)
        {
            Slot.TimePlayed.Append(FString::FromInt(InTime) + TimeAboveOneString);
        }
        else
        {
            Slot.TimePlayed.Append(FString::FromInt(InTime) + TimeEqualsOneString);
        }
    }
}

bool SaveGameManager::FindAndRenameSlot(const FString& SaveSlotName, const FString& NewSlotName)
{
    for (auto& Slot : SaveData.Slots)
    {
        if (Slot.Key == SaveSlotName)
        {
            Slot.Key = NewSlotName;
            return true;
        }
    }
    return false;
}

void SaveGameManager::SpawnTimeKeeper()
{
    if (World)
    {
        TimeKeeper = World->SpawnActor<ATimeKeeper>();
        TimeKeeper->Init(&CurrentData, bIsVerboseLoggingEnabled);
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error creating TimeKeeper: World is not set"));
    }
}

void SaveGameManager::SpawnActors(UWorld* InWorld, const TArray<FActorSaveData>& ActorData, TArray<AActor*>& SpawnedActors)
{
    FActorSpawnParameters Params;
    for (const auto& Data : ActorData)
    {
        if (!Data.Class)
        {
            UE_LOG(LogSaveGame, Warning, TEXT("Error spawning stored actor: class is invalid"));
            continue;
        }

        Params.SpawnCollisionHandlingOverride = Data.SpawnMethod;
        AActor* SpawnedActor = InWorld->SpawnActor<AActor>(Data.Class, Data.Transform, Params);
        ISaveGameEvents* Interface = Cast<ISaveGameEvents>(SpawnedActor);
        if (Interface)
        {
            Interface->PreLoad();
        }

        //load physics state for any skeletal mesh or static mesh components
        if (Data.bIsSimulatingPhysics)
        {
            const TSet<UActorComponent*>& Components = SpawnedActor->GetComponents();
            for (const auto& Component : Components)
            {
                if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Component))
                {
                    Mesh->SetSimulatePhysics(Data.bIsSimulatingPhysics);
                }
                else if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
                {
                    StaticMesh->SetSimulatePhysics(Data.bIsSimulatingPhysics);
                }
            }
        }

        if (Interface)
        {
            Interface->PostLoad();
        }
        SpawnedActors.Add(SpawnedActor);
    }
}

void SaveGameManager::SpawnObjects(TArray<UObject*>& SpawnedObjects)
{
    FActorSpawnParameters Params;
    for (const auto& Object : CurrentData.Objects)
    {
        if (!Object.Value)
        {
            UE_LOG(LogSaveGame, Warning, TEXT("Error spawning stored actor: class is invalid"));
            continue;
        }

        UObject* SpawnedObject = NewObject<UObject>(Object.Value);
        ISaveGameEvents* Interface = Cast<ISaveGameEvents>(SpawnedObject);

        if (Interface)
        {
            Interface->PostLoad();
        }
        SpawnedObjects.Add(SpawnedObject);
    }
}

void SaveGameManager::SetTimeAndDateCreated(FSaveSlotInfo& Slot)
{
    FDateTime CurrentTime;
    const FDateTime& Today = CurrentTime.Today();
    const FString& Date = Today.ToString();
    const FString& Year = Date.Mid(0, 4); // 4 for the year
    const FString& Month = Date.Mid(5, 2);// 2 for the month
    const FString& Day = Date.Mid(8, 2);  // 2 for the day
    Slot.DateCreated.Reserve(10); // 2 for the hypens
    Slot.DateCreated.Append(Month + '-' + Day + '-' + Year);

    //get the hour by normal time because some people have trouble reading military time
    int32 Hour = Today.Now().GetHour12();
    int32 Minute = Today.Now().GetMinute();
    //if the minute counter is at least 10
    if (Minute > 9)
    {
        //add minutes as they are
        Slot.TimeCreated = FString::FromInt(Hour) + ":" + FString::FromInt(Minute);
    }
    else
    {
        //add 0 for a cleaner output
        //Example: 12:05 vs 12:5
        Slot.TimeCreated = FString::FromInt(Hour) + ":0" + FString::FromInt(Minute);
    }

    const FDateTime& Now = Today.Now();
    const int32 Hour24 = Now.GetHour();
    //a 24 hour clock starts at 0, so 11 is noon, which is the first minute of the day to use PM
    if (Hour24 < 11)
    {
        Slot.TimeCreated.Append("AM");
    }
    else
    {
        Slot.TimeCreated.Append("PM");
    }
}