// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    ExpandedSaveGameLibrary.h: High-level interface that retrieves values from
    Project Settings and works with the SaveGameManager.
=============================================================================*/

#include "ExpandedSaveGameLibrary.h"
#include "SaveGameLibraryModule.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/Paths.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/GameplayStatics.h"
#include "GeneralProjectSettings.h"

#include "Utils/LogDeclarations.h"
#include "SaveSystemSettings.h"
#include "InternalSystem/SaveGameManager.h"
#include "InternalSystem/SaveGameEvents.h"

//TODO: look into storing actors in archives to serialize variables

#define READ_SETTING(Var, Val, func, Message)                                                        \
if (Var != Val)                                                                                      \
{                                                                                                    \
    Var = Val;                                                                                       \
    func(Var);                                                                                       \
    UE_LOG(LogSaveGame, Display, TEXT("%s%s"), *Message, (Val ? TEXT("enabled") : TEXT("disabled")));\
}

#define STORE_VALUE(func, type)                                           \
void UExpandedSaveGameLibrary::Store##func(const FString Name, type Value)\
{                                                                         \
    Manager.Store##func(Name, Value);                                     \
}

#define GET_ARRAY(func)                                             \
for (int32 Index = 0; Index < ArraySize; ++Index)                   \
{                                                                   \
    const FString& ValueName = ArrayName + FString::FromInt(Index); \
    StoredArray.Add(Manager.Get##func(ValueName));                  \
}

#define GETTER(func, type, line, ...)                      \
void UExpandedSaveGameLibrary::GetStored##func(__VA_ARGS__)\
{                                                          \
    line;                                                  \
}

#define GET_VALUE(func, type) GETTER(func, type, Value = Manager.Get##func(Name), const FString Name, type& Value)
#define GET_VALUE_ARRAY(func, type) GETTER(func##Array, type, GET_ARRAY(func), const FString ArrayName, const int32 ArraySize, TArray<type>& StoredArray)

#define GENERATE_GETTERS(func, type)\
GET_VALUE(func, type)\
GET_VALUE_ARRAY(func, type)

static SaveGameManager Manager;
static UWorld* CurrentWorld;
static TSubclassOf<USaveGameObject> SaveObjectClass;
static FString SaveFileName;
//initialize bools to default values in settings to prevent invalid settings read
static bool bIsVerboseLoggingEnabled = true;
static bool bIsMultithreading = true;
static bool bIsMultithreadingEvents = false;
static bool bIsUsingSingleSaveFile = true;
static bool bIsActorSavingAutomatic = false;

//internal function to detect if a save file exists
bool DetectSaveFile();
//internal function to retrieve settings set in Project Settings under Plugins->Save Game System
void ReadSettings();

UExpandedSaveGameLibrary::UExpandedSaveGameLibrary(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ReadSettings();
}

void UExpandedSaveGameLibrary::SaveCurrentLevelForActiveSlot(UObject* Context)
{
    const FString& LevelName = UGameplayStatics::GetCurrentLevelName(Context->GetWorld());
    Manager.StoreString("Current Level", LevelName);
    if (bIsVerboseLoggingEnabled)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully saved current level as %s"), *LevelName);
    }
}

void UExpandedSaveGameLibrary::LoadLevelForActiveSlot(UObject* Context)
{
    const FString& Level = Manager.GetString("Current Level");
    if (!Level.IsEmpty())
    {
        UGameplayStatics::OpenLevel(Context->GetWorld(), *Level);
        if (bIsVerboseLoggingEnabled)
        {
            UE_LOG(LogSaveGame, Display, TEXT("Succesfully loaded level for save slot %s"), *Manager.GetCurrentSlotName());
        }
    }
    else
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to load level for save slot %s: no stored level found"), *Manager.GetCurrentSlotName());
    }
}

//Generate functions to reduce line count
STORE_VALUE(Int, const int)
STORE_VALUE(Float, const float)
STORE_VALUE(Boolean, const bool) //i decided to call it Boolean instead of Bool so that developers wouldnt have to replace every StoreBoolean node
STORE_VALUE(String, const FString)
STORE_VALUE(Name, const FName)
STORE_VALUE(Text, const FText)
STORE_VALUE(Class, const TSubclassOf<UObject>)
STORE_VALUE(Object, UObject* const)
STORE_VALUE(Vector, const FVector)
STORE_VALUE(Rotator, const FRotator)
STORE_VALUE(Transform, const FTransform)

//GET_VALUE(Class, TSubclassOf<UObject>)
GENERATE_GETTERS(Int, int)
GENERATE_GETTERS(Float, float)
GENERATE_GETTERS(Boolean, bool)
GENERATE_GETTERS(Class, TSubclassOf<UObject>)
GENERATE_GETTERS(ActorData, FActorSaveData)
GENERATE_GETTERS(Vector, FVector)
GENERATE_GETTERS(Rotator, FRotator)
GENERATE_GETTERS(Transform, FTransform)
GENERATE_GETTERS(String, FString)
GENERATE_GETTERS(Text, FText)
GENERATE_GETTERS(Name, FName)

//StoreActor has an additional parameter, so it cant be used in the macro
void UExpandedSaveGameLibrary::StoreActor(const FString ObjectName, AActor* const ActorToStore, ESpawnActorCollisionHandlingMethod SpawnMethod)
{
    Manager.StoreActor(ObjectName, ActorToStore, SpawnMethod);
}

void UExpandedSaveGameLibrary::UpdateSaveSlotProjectVersion(const FString SaveSlot, const FString NewVersion, bool& bSuccess)
{
    bSuccess = Manager.UpdateSaveSlotProjectVersion(SaveSlot, NewVersion);
}

bool UExpandedSaveGameLibrary::EraseStoredValue(const FString VariableName)
{
    return Manager.EraseValue(VariableName);
}

void UExpandedSaveGameLibrary::GetSaveSlotNames(TArray<FString>& SlotNames, const int UserIndex)
{
    if (!DetectSaveFile())
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to get save slot names: no save file detected"));
        return;
    }
    Manager.LoadData(SaveFileName, UserIndex);
    SlotNames = Manager.GetSaveSlotNames();
    if (bIsVerboseLoggingEnabled && SlotNames.Num() == 0)
    {
        UE_LOG(LogSaveGame, Warning, TEXT("Unable to get save slot names: no save slots detected"));
    }
}

void UExpandedSaveGameLibrary::GetActorNames(TArray<FString>& ActorNames)
{
    ActorNames = Manager.GetActorNames();
}

void UExpandedSaveGameLibrary::GetDateCreated(FString& Date)
{
    Date = Manager.GetDateCreated();
}

void UExpandedSaveGameLibrary::GetTimePlayed(const FString SaveSlotName, FString& TimePlayed)
{
    TimePlayed = Manager.GetTimePlayed(SaveSlotName);
}

void UExpandedSaveGameLibrary::GetSaveSlotProjectVersion(const FString SaveSlotName, FString& ProjectVersion)
{
    ProjectVersion = Manager.GetSaveSlotProjectVersion(SaveSlotName);
}

void UExpandedSaveGameLibrary::GetSaveSlotInfo(const FString SaveSlotName, FSaveSlotInfo& SaveSlotInfo)
{
    SaveSlotInfo = Manager.GetSaveSlotInfo(SaveSlotName);
}

void UExpandedSaveGameLibrary::GetActiveSaveSlot(FString& ActiveSlot)
{
    ActiveSlot = Manager.GetCurrentSaveSlot();
}

bool UExpandedSaveGameLibrary::DoSaveSlotsExist()
{
    if (!bIsUsingSingleSaveFile)
    {
        return DetectSaveFile();
    }
    //check for a save file or an existing save slot in data that hasn't be written to a file yet
    return !Manager.GetCurrentSaveSlot().IsEmpty() || !Manager.GetSaveSlotNames().IsEmpty();
}

bool UExpandedSaveGameLibrary::DoesSaveSlotExit(const FString SaveSlotName)
{
    return Manager.DoesSaveSlotExist(SaveSlotName);
}

void UExpandedSaveGameLibrary::SpawnStoredActor(UObject* Context, const FString ActorName, AActor*& SpawnedActor)
{
    CurrentWorld = Context->GetWorld();
    if (!CurrentWorld)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error spawning stored actor: cannot get World from where SpawnStoredActor is being called from"));
        return;
    }

    SpawnedActor = Manager.SpawnActor(ActorName);
}

void UExpandedSaveGameLibrary::SpawnStoredActors(UObject* Context, TArray<AActor*>& SpawnedActors)
{
    CurrentWorld = Context->GetWorld();
    if (!CurrentWorld)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error spawning stored actors: cannot get World from where SpawnStoredActors is being called from"));
        return;
    }

    SpawnedActors = Manager.SpawnStoredActors(CurrentWorld);
}

void UExpandedSaveGameLibrary::SpawnStoredObject(const FString ObjectName, UObject* Outer, UObject*& SpawnedObject)
{
    SpawnedObject = Manager.SpawnObject(ObjectName, Outer);
}

void UExpandedSaveGameLibrary::SpawnStoredObjects(UObject* Context, TArray<UObject*>& SpawnedObjects)
{
    CurrentWorld = Context->GetWorld();
    if (!CurrentWorld)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error spawning stored objects: cannot get World from where SpawnStoredObjects is being called from"));
        return;
    }

    SpawnedObjects = Manager.SpawnStoredObjects(CurrentWorld);
}

void UExpandedSaveGameLibrary::SetActiveSaveSlot(const FString SaveSlotName)
{
    Manager.SetActiveSave(SaveSlotName);
}

void UExpandedSaveGameLibrary::SetIsTimeKeeperEnabled(UObject* Context, bool bNewState)
{
    Manager.SetIsTimeKeeperEnabled(Context, bNewState);
}

void UExpandedSaveGameLibrary::SaveGame(UObject* Context, FString SaveSlotName, int UserIndex)
{
    CurrentWorld = Context->GetWorld();
    if (!CurrentWorld)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error spawning stored actors: cannot get World from the blueprint that SpawnStoredActors is being called from"));
        return;
    }

    if (bIsActorSavingAutomatic)
    {
        TArray<AActor*> ActorsWithInterface;
        UGameplayStatics::GetAllActorsWithInterface(CurrentWorld, USaveGameEvents::StaticClass(), ActorsWithInterface);
        for (AActor* Actor : ActorsWithInterface)
        {
            Manager.StoreActor(Actor->GetDebugName(Actor), Actor, ESpawnActorCollisionHandlingMethod::Undefined);
        }
    }

    ReadSettings();
    Manager.SetWorld(CurrentWorld);

    if (SaveObjectClass)
    {
        Manager.SaveGame(SaveSlotName, UserIndex, SaveObjectClass, bIsMultithreading);
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("Unable to save game: SaveGameClass is likely not set in ProjectSettings under Plugins->SaveGameSettings"));
    }
}

bool UExpandedSaveGameLibrary::LoadGame(UObject* Context, const FString SaveSlotName, int UserIndex)
{
    UE_LOG(LogSaveGame, Display, TEXT("Attempting to load slot %s for player %d"), *SaveSlotName, UserIndex);

    if (!DetectSaveFile())
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error loading game: no save file detected"));
        return false;
    }
    CurrentWorld = Context->GetWorld();
    if (!CurrentWorld)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error loading game: Unable to get world from where LoadGame is being called from"));
        return false;
    }

    ReadSettings();
    Manager.SetWorld(CurrentWorld);

    if (Manager.LoadGame(SaveSlotName, UserIndex))
    {
        return true;
    }
    return false;
}

bool UExpandedSaveGameLibrary::LoadData(UObject* Context, const int UserIndex)
{
    return Manager.LoadData(SaveFileName, UserIndex);
}

void UExpandedSaveGameLibrary::NewSaveGame(UObject* Context, const FString SaveSlotName, bool bOverwriteExistingSave, bool& bSuccess)
{
    bSuccess = false;
    if (!Context)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error creating new game: Context that NewSaveGame is being called from is not valid"));
        return;
    }

    CurrentWorld = Context->GetWorld();
    if (!CurrentWorld)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Error creating new game: Unable to get world from blueprint NewSaveGame is being called from"));
        return;
    }

    ReadSettings();
    Manager.SetWorld(CurrentWorld);
    bSuccess = Manager.NewGame(SaveSlotName, SaveObjectClass, bIsMultithreading, bOverwriteExistingSave);
}

bool UExpandedSaveGameLibrary::DeleteGame(const FString SaveSlotName, const int UserIndex)
{
    UE_LOG(LogSaveGame, Display, TEXT("Attempting to delete save slot %s for player %d"), *SaveSlotName, UserIndex);
    if (Manager.DeleteGame(SaveSlotName, UserIndex, SaveObjectClass, bIsMultithreading))
    {
        UE_LOG(LogSaveGame, Display, TEXT("Successfully deleted save slot %s for player %d"), *SaveSlotName, UserIndex);
        return true;
    }
    return false;
}

void UExpandedSaveGameLibrary::RenameSaveSlot(UObject* Context, const FString SaveSlotName, const int UserIndex, const FString NewSlotName, bool& bSuccess)
{
    bSuccess = Manager.RenameSlot(SaveSlotName, NewSlotName);
}

void UExpandedSaveGameLibrary::ParseProperty(const FString& ObjectName, FProperty* Property, void* ValuePtr, FString&& PropertyName)
{
    PropertyName.Append(Property->GetAuthoredName());

    if (StoreProperty(Property, ValuePtr, PropertyName))
    {
        return;
    }
    else if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
    {
        FScriptArrayHelper Helper(ArrayProperty, ValuePtr);
        const int32 ArraySize = Helper.Num();
        for (int32 Index = 0; Index < ArraySize; ++Index)
        {
            //name of the value at array index is ArrayName + ArrayIndex
            FString Name = PropertyName + FString::FromInt(Index);
            StoreProperty(ArrayProperty->Inner, Helper.GetRawPtr(Index), MoveTemp(Name));
        }
    }
    else if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
    {
        for (TFieldIterator<FProperty> Iterator(StructProperty->Struct); Iterator; ++Iterator)
        {
            //append here so that the first value will also include object name
            //const FString& ObjectId = ObjectName.Mid(ObjectName.Len() - 3);
            if (!PropertyName.Contains(ObjectName))
            {
                PropertyName.Append(ObjectName);
            }
            for (int32 ArrayIndex = 0; ArrayIndex < Iterator->ArrayDim; ArrayIndex++)
            {
                void* StructPtr = Iterator->ContainerPtrToValuePtr<void>(ValuePtr, ArrayIndex);
                //name of the property is StructName + ObjectName + PropertyName
                ParseProperty(ObjectName, *Iterator, StructPtr, MoveTemp(PropertyName));
                //reset name after parsing property so there is no overlap
                FProperty* Inner = CastField<FProperty>(*Iterator);
                PropertyName.RemoveFromEnd(Inner->GetAuthoredName());
            }
        }
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("Unable to save data for value %s: value type could not be determined"), *Property->GetAuthoredName());
    }
}

bool UExpandedSaveGameLibrary::StoreProperty(FProperty* Property, void* ValuePtr, const FString& PropertyName)
{
    if (FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
    {
        if (NumericProperty->IsFloatingPoint())
        {
            const float FloatValue = NumericProperty->GetFloatingPointPropertyValue(ValuePtr);
            Manager.StoreFloat(PropertyName, FloatValue);
            return true;
        }
        else if (NumericProperty->IsInteger())
        {
            const int IntValue = NumericProperty->GetSignedIntPropertyValue(ValuePtr);
            Manager.StoreInt(PropertyName, IntValue);
            return true;
        }
    }
    else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
    {
        const bool BoolValue = BoolProperty->GetPropertyValue(ValuePtr);
        Manager.StoreBoolean(PropertyName, BoolValue);
        return true;
    }
    else if (FNameProperty* NameProperty = CastField<FNameProperty>(Property))
    {
        const FName& NameValue = NameProperty->GetPropertyValue(ValuePtr);
        Manager.StoreName(PropertyName, NameValue);
        return true;
    }
    else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
    {
        const FString& StringValue = StringProperty->GetPropertyValue(ValuePtr);
        Manager.StoreString(PropertyName, StringValue);
        return true;
    }
    else if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
    {
        const FText& TextValue = TextProperty->GetPropertyValue(ValuePtr);
        Manager.StoreText(PropertyName, TextValue);
        return true;
    }
    return false;
}

bool DetectSaveFile()
{
    TArray<FString> Files;
    FFileManagerGeneric FileManager;
    FString Directory = FPaths::ProjectSavedDir() + "SaveGames/";
    FileManager.FindFiles(Files, *Directory);
    return Files.Num() > 0;
}

void ReadSettings()
{
    UE_LOG(LogSaveGame, Display, TEXT("Checking Expanded Save Game Library settings"));
    const USaveGameSystem_Settings* Plugin = GetMutableDefault<USaveGameSystem_Settings>();
    if (!Plugin)
    {
        UE_LOG(LogSaveGame, Error, TEXT("Failed to initialize SaveGameSystem: Plugin was not added"));
        return;
    }

    //if class has been changed in project settings, change it here
    if (SaveObjectClass != Plugin->SaveGameObjectClass)
    {
        SaveObjectClass = Plugin->SaveGameObjectClass;
    }
    if (SaveObjectClass)
    {
        UE_LOG(LogSaveGame, Display, TEXT("Save Game Object Class is %s"), *SaveObjectClass->GetName());
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("SaveGameObjectClass is invalid. It is likely not set in Project Settings under Plugins->Save Game Settings"));
        return;
    }

    if (!SaveFileName.IsEmpty())
    {
        UE_LOG(LogSaveGame, Display, TEXT("SaveFileName is set to %s"), *SaveFileName);
    }
    else
    {
        UE_LOG(LogSaveGame, Error, TEXT("Failed to initialize save game system: Save File Name is empty in Project Settings under Plugins->Save Game Settings"));
        return;
    }
    Manager.SetSaveFileName(SaveFileName);

    //if file name has been changed in project settings, change it here
    if (SaveFileName != Plugin->SaveFileName)
    {
        SaveFileName = Plugin->SaveFileName;
    }
    if (SaveFileName.IsEmpty())
    {
        if (const UGeneralProjectSettings* ProjectSettings = GetDefault<UGeneralProjectSettings>())
        {
            SaveFileName = ProjectSettings->ProjectName;
        }
        else
        {
            UE_LOG(LogSaveGame, Error, TEXT("Error retrieving project name from project settings: SaveFileName is empty and the system will not work"));
        }
    }

    if (bIsActorSavingAutomatic != Plugin->bAutomaticActorSaving)
    {
        bIsActorSavingAutomatic = Plugin->bAutomaticActorSaving;
        UE_LOG(LogSaveGame, Display, TEXT("Automatic Actor Saving is %s"), (bIsActorSavingAutomatic ? TEXT("enabled") : TEXT("disabled")));
    }

    auto ReadSetting = [](bool& OutResult, bool bIsEnabled, const TFunctionRef<void(bool OutResult)>& func, const FString& Message)
    {
        if (OutResult != bIsEnabled)
        {
            OutResult = bIsEnabled;
            func(OutResult);
            UE_LOG(LogSaveGame, Display, TEXT("%s is %s"), *Message, (bIsEnabled ? TEXT("enabled") : TEXT("disabled")));
        }
    };
    READ_SETTING(bIsVerboseLoggingEnabled,  Plugin->bVerboseLogging,        Manager.SetIsVerboseLoggingEnabled, FString("Verbose Logging is "));
    READ_SETTING(bIsMultithreading,         Plugin->bMultithreadedSaving,   Manager.SetIsMultithreading,        FString("Multithreaded Saving is "));
    READ_SETTING(bIsMultithreadingEvents,   Plugin->bMultithreadedEvents,   Manager.SetIsMultithreadingEvents,  FString("Multithreaded Events are "));
    READ_SETTING(bIsUsingSingleSaveFile,    Plugin->bSingleFileSaving,      Manager.SetIsUsingSingleSaveFile,   FString("Single File Saving is "));
    UE_LOG(LogSaveGame, Display, TEXT("Finished checking Expanded Save Game Library settings"));
}

/////////////////////////////////////////////////////////////////////////////////
//	Everything below this line is a work in progress and not currently used
/////////////////////////////////////////////////////////////////////////////////

#if 0
void UExpandedSaveGameLibrary::GetStoredStruct(UObject* Context, const FString StructName, const TArray<FString> StructVariables, UStruct*& StoredStruct)
{
    //member variables are retrived by StructName + ObjectName + VariableName
    FString Name = StructName + Context->GetName();
    for (const auto& Variable : StructVariables)
    {
        const FString& StructVariable = Name + Variable;
        FProperty* Property;
        StoredStruct->AddCppProperty();
    }
}
#endif