// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	ExpandedSaveGameLibrary.h: Blueprint function library to interface with the	
	SaveGameManager.
=============================================================================*/

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "GenericPlatform/GenericPlatform.h"
#include "Structs/ActorSaveData.h"
#include "Structs/SaveSlotInfo.h"
#include "Structs/SaveGameData.h"

#include "ExpandedSaveGameLibrary.generated.h"

/*
*	Function library class.
*	Each function in it is expected to be static and represents blueprint node that can be called in any blueprint.
*
*	When declaring function you can define metadata for the node. Key function specifiers will be BlueprintPure and BlueprintCallable.
*	BlueprintPure - means the function does not affect the owning object in any way and thus creates a node without Exec pins.
*	BlueprintCallable - makes a function which can be executed in Blueprints - Thus it has Exec pins.
*	DisplayName - full name of the node, shown when you mouse over the node and in the blueprint drop down menu.
*				Its lets you name the node using characters not allowed in C++ function names.
*	CompactNodeTitle - the word(s) that appear on the node.
*	Keywords -	the list of keywords that helps you to find node when you search for it using Blueprint drop-down menu.
*				Good example is "Print String" node which you can find also by using keyword "log".
*	Category -	the category your node will be under in the Blueprint drop-down menu.
*
*	For more info on custom blueprint nodes visit documentation:
*	https://wiki.unrealengine.com/Custom_Blueprint_Node_Creation
*/

UCLASS()
class UExpandedSaveGameLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	//////////////////////////////////////////////////////////////////
	//	Level Saving and Loading
	//////////////////////////////////////////////////////////////////

	/* Saves the current level under the active save slot by storing the active level as a string under the name "Current Level"
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void SaveCurrentLevelForActiveSlot(UObject* Context);

	/* Loads the level stored under the active save slot by calling OpenLevel and using the name stored as a string under the name "Current Level"
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void LoadLevelForActiveSlot(UObject* Context);

	//////////////////////////////////////////////////////////////////
	//	Store Values
	//////////////////////////////////////////////////////////////////

	/* Stores 'Value' under 'VariableName' that can be loaded using GetStoredInt
	@param VariableName			The name to associate 'Value' with
	@param Value				The value to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreInt(const FString VariableName, const int Value);
	
	/* Stores 'Value' under 'VariableName' that can be loaded using GetStoredFloat
	@param VariableName			The name to associate 'Value' with
	@param Value				The value to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreFloat(const FString VariableName, const float Value);
	
	/* Stores 'Value' under 'VariableName' that can be loaded using GetStoredBoolean
	@param VariableName			The name to associate 'Value' with
	@param Value				The value to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreBoolean(const FString VariableName, const bool Value);
	
	/* Stores 'String' under 'StringLabel' that can be loaded using GetStoredString
	@param StringLabel			The name to associate 'String' with
	@param String				The string to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreString(const FString StringLabel, const FString String);

	/* Stores 'Name' under 'NameLabel' that can be loaded using GetStoredName
	@param StringName			The name to associate 'Name' with
	@param String				The name to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreName(const FString NameLabel, const FName Name);

	/* Stores 'Text' under 'TextLabel' that can be loaded using GetStoredText
	@param StringName			The name to associate 'Text' with
	@param String				The text to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreText(const FString TextLabel, const FText Text);

	/* Stores 'Class' under 'ClassName' that can be loaded using GetStoredClass
	@param ClassName			The name to associate 'Class' with
	@param Class				The class to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreClass(const FString ClassName, const TSubclassOf<UObject> Class);

	/* Stores 'ObjectToStore' under 'ObjectName' that can be loaded using GetStoredObject.
	If ObjectName is empty, the world name of the object is used instad.
	Objects stored this way are automatically spawned by calling SpawnStoredObjects unless they are erased using EraseStoredValue
	@param ObjectName			The name to associate 'ObjectToStore' with
	@param ObjectToStore		The object to save the class for
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreObject(const FString ObjectName, UObject* const ObjectToStore);

	/* Stores 'ActorToStore' under 'ActorName' using the ActorSaveData struct.
	If ActorName is empty, the world name of the actor is used instead.
	Actors stored this way are automatically spawned by calling SpawnStoredActors unless they are erased using EraseStoredValue
	@param ActorName			The name to store the actor under. If empty, the world name of the actor is used instead.
	@param ActorToStore			The actor to create save data for
	@param SpawnMethod			The collision handling method that will be used when this actor is spawned using SpawnStoredActors
	*/
	UFUNCTION(BlueprintCallable, meta = (AutoCreateRefTerm="VariablesToSave", AdvancedDisplay="SpawnMethod, bDoStoreComponents"), Category = "Expanded Save Game Library")
	static void StoreActor(const FString ActorName, AActor* const ActorToStore, ESpawnActorCollisionHandlingMethod SpawnMethod);
	
	/* Stores 'Vector' under 'VectorName' that can be loaded using GetStoredVector
	@param VectorName			The name to associate 'Vector' with
	@param Vector				The vector to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreVector(const FString VectorName, const FVector Vector);

	/* Stores 'Rotator' under 'RotatorName' that can be loaded using GetStoredRotator
	@param RotatorName			The name to associate 'Rotator' with
	@param Rotator				The rotator to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreRotator(const FString RotatorName, const FRotator Rotator);
	
	/* Stores 'Transform' under 'TransformName' that can be loaded using GetStoredTransform
	@param TransformName		The name to associate 'Transform' with
	@param Transform			The transform to store
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void StoreTransform(const FString TransformName, const FTransform Transform);

	/* Updates the project version currently stored on the SaveSlot
	ProjectVersion is stored as a string because it uses multiple decimals
	@param SaveSlot The save slot to update the project version for
	@param NewVersion The new project version to use for the save slot
	@param bSuccess True if the save slot exists, otherwise false
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void UpdateSaveSlotProjectVersion(const FString SaveSlot, const FString NewVersion, bool& bSuccess);

	//////////////////////////////////////////////////////////////////
	//	Erase values
	//////////////////////////////////////////////////////////////////
	
	/* Delete the save data for VariableName from the current save slot
	@param VariableName		The name of the value to delete
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static bool EraseStoredValue(const FString VariableName);

	//////////////////////////////////////////////////////////////////
	//	Getters
	//////////////////////////////////////////////////////////////////

	/* Retreive the class stored by ClassName
	@param ClassName		The name of the class to retrieve
	@param StoredClass		The class that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredClass(const FString ClassName, TSubclassOf<UObject>& StoredClass);

	/* Retreive the class array stored by ArrayName
    @param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
    @param StoredArray		The array of classes that was stored
    */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredClassArray(const FString ArrayName, const int32 ArraySize, TArray<TSubclassOf<UObject>>& StoredArray);

	/* Retreive the integer value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredInt		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredInt(const FString VariableName, int& StoredInt);

	/* Retreive the integer array stored by ArrayName
    @param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of integers that was stored
    */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredIntArray(const FString ArrayName, const int32 ArraySize, TArray<int>& StoredArray);

	/* Retreive the float value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredFloat		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredFloat(const FString VariableName, float& StoredFloat);

	/* Retreive the float array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of floats that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredFloatArray(const FString ArrayName, const int32 ArraySize, TArray<float>& StoredArray);

	/* Retreive the boolean value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredBoolean	The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredBoolean(const FString VariableName, bool& StoredBoolean);

	/* Retreive the boolean array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of booleans that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredBooleanArray(const FString ArrayName, const int32 ArraySize, TArray<bool>& StoredArray);

	/* Retreive the ActorSaveData stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param ActorData		The data that was stored for the actor
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredActorData(const FString ObjectName, FActorSaveData& ActorData);

	/* Retreive the ActorSaveData array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of actor data that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredActorDataArray(const FString ArrayName, const int32 ArraySize, TArray<FActorSaveData>& StoredArray);

	/* Retreive the Vector value stored by VectorName
	@param VectorName		The name of the value to retrieve
	@param StoredVector		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredVector(const FString VectorName, FVector& StoredVector);

	/* Retreive the Vector array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of vectors that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredVectorArray(const FString ArrayName, const int32 ArraySize, TArray<FVector>& StoredArray);

	/* Retreive the Rotator value stored by RotatorName
	@param RotatorName		The name of the value to retrieve
	@param StoredVector		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredRotator(const FString RotatorName, FRotator& StoredRotator);

	/* Retreive the Rotator array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of rotators that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredRotatorArray(const FString ArrayName, const int32 ArraySize, TArray<FRotator>& StoredArray);

	/* Retreive the Transform value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredTransform	The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredTransform(const FString TransformName, FTransform& StoredTransform);

	/* Retreive the Transform array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of transforms that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredTransformArray(const FString ArrayName, const int32 ArraySize, TArray<FTransform>& StoredArray);

	/* Retreive the String value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredString		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredString(const FString StringLabel, FString& StoredString);

	/* Retreive the String array stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of strings that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredStringArray(const FString ArrayName, const int32 ArraySize, TArray<FString>& StoredArray);

	/* Retreive the Text value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredText		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredText(const FString TextLabel, FText& StoredText);

	/* Retreive the Text array stored by ArrayName
	@param VariableName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of texts that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredTextArray(const FString ArrayName, const int32 ArraySize, TArray<FText>& StoredArray);

	/* Retreive the Name value stored by VariableName
	@param VariableName		The name of the value to retrieve
	@param StoredName		The value that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredName(const FString NameLabel, FName& StoredName);

	/* Retreive the Name aarray stored by ArrayName
	@param ArrayName		The name of the array to retrieve
	@param ArraySize		The number of elements in the array
	@param StoredArray		The array of names that was stored
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetStoredNameArray(const FString ArrayName, const int32 ArraySize, TArray<FName>& StoredArray);

	/* Get all names of existing save slots
	returns array of strings, where each string is a save slot name
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetSaveSlotNames(TArray<FString>& SlotNames, const int UserIndex);
	
	/* Get all names of stored actors */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetActorNames(TArray<FString>& ActorNames);
	
	/* Get the date that the current save slot was created as a string
	Format: Year-Month-Day
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetDateCreated(FString& Date);
	
	/* Get the total amount of time that this save slot has been played
	Format: Years, Months, Weeks, Days, Hours, Minutes, Seconds
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetTimePlayed(const FString SaveSlotName, FString& TimePlayed);
	
	/* Get project version that SaveSlotName was last saved with
	@param SaveSlotName The name of the save slot to retrieve the project version for
	@param ProjectVersion The version of the project that SaveSlotName was last saved with
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetSaveSlotProjectVersion(const FString SaveSlotName, FString& ProjectVersion);

	/* Get the info for the current save slot as a struct
	Includes: SaveSlotName, DateCreated, TimeCreated, TimePlayed
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetSaveSlotInfo(const FString SaveSlotName, FSaveSlotInfo& SaveSlotInfo);

	/* Get the name of the active save slot were data is currently being stored */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static void GetActiveSaveSlot(FString& ActiveSlotName);

	/* Detects if any save slots exist in the save games folder in the directory /Project/Saved/SaveGames/
	Detects if a save slot exists that hasn't been written to a file yet
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static bool DoSaveSlotsExist();

	/* Checks to see if SaveSlotName has been created. The save slot does not have to be saved to a file to be detected 
	@param SaveSlotName			The name of the save slot to look for
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Expanded Save Game Library")
	static bool DoesSaveSlotExit(const FString SaveSlotName);

	//////////////////////////////////////////////////////////////////
	//	End of Getters
	//////////////////////////////////////////////////////////////////

	/* Spawns a single actor saved with the StoreActor function. 
	@param SpawnedActor			The stored actor that was successfully spawned. If it doesn't exist, it will be null
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void SpawnStoredActor(UObject* Context, const FString ActorName, AActor*& SpawnedActor);

	/* Spawns all actors saved with the StoreActor function. 
	WARNING: This function cannot be called from PostLoad if Multithreaded Events is enabled. It will halt execution of the program
	@param SpawnedActors		The stored actors that were succesfully spawned, if any
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void SpawnStoredActors(UObject* Context, TArray<AActor*>& SpawnedActors);
	
	/* Spawn the UObject stored by ObjectName
	@param ObjectName		The name of the object to spawn
	@param Outer			The owner of the object
	@param SpawnedObject	The object that is spawned
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void SpawnStoredObject(const FString ObjectName, UObject* Outer, UObject*& SpawnedObject);

	/* Spawns all objects saved with the StoreObject function
	@param SpawnedObjects		The stored objects that were successfully spawned
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void SpawnStoredObjects(UObject* Context, TArray<UObject*>& SpawnedObjects);

	/* Sets the currently active save slot to 'SaveSlotName' so that any stored values are stored under 'SaveSlotName'. If 'SaveSlotName' is empty, the active slot will be set to none
	* does NOT call LoadData internally
	@param SaveSlotName			The name of the slot to make active
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static void SetActiveSaveSlot(const FString SaveSlotName);

	/* Enables or disables the tracking of time for the active save slot
	@param bNewState			If true, keeps track of time for the active save slot. If false, pauses the time for the active save slot
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void SetIsTimeKeeperEnabled(UObject* Context, bool bNewState);

	//////////////////////////////////////////////////////////////////
	//	File Reading/Writing
	//////////////////////////////////////////////////////////////////
	
	/* Creates a save slot under SaveSlotName for the player at UserIndex. 
	* The .sav file is saved with the SaveFileName specified in Project Settings under Plugins->Save Game Settings
	* If the .sav file doesn't exist, it is created under /ProjectName/Saved/SaveGames
	* Calls LoadData internally if data hasn't been loaded yet.
	
	WARNING: This function cannot be multithreaded
	@param SaveSlotName			The name that the save slot is stored as
	@param UserIndex			The index of the player to save the game for
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void SaveGame(UObject* Context, const FString SaveSlotName, const int UserIndex);
	
	/* Attempts to load the game under SaveSlotName for player at UserIndex. Does not spawn actors. You must call SpawnStoredActor or SpawnStoredActors
	* Calls LoadData internally if data hasn't been loaded yet.
	
	WARNING: This function cannot be multithreaded
	@param SaveSlotName			The name of the save slot to load
	@param UserIndex			The index of the player to load the game for
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static bool LoadGame(UObject* Context, const FString SaveSlotName, const int UserIndex);
	
	/* Reads the data from the save file specified under ProjectSettings in Plugins->Save Game Settings
	@param UserIndex			The index of the player to load the data for
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static bool LoadData(UObject* Context, const int UserIndex);

	/* Creates a new save game slot with the provided SaveSlotName for player at UserIndex. 
	* If one already exists, it is deleted first if OverwriteExistingSave is true.
	* does NOT call SaveGame interally so that you can add data before saving
	* calls LoadData internally if data hasn't been loaded yet.
	
	WARNING: This function cannot be multithreaded
	@param Context					The object that is used to retrieve the world from. This defaults to the Actor that is calling it
	@param SaveSlotName				The name to assocate this SaveGame with. This will also be name of the .sav file
	@param UserIndex				The index of the player to create a new save for
	@param bOverwriteExistingSave	Whether or not to overwrite SaveSlotName if it already exists
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void NewSaveGame(UObject* Context, const FString SaveSlotName, bool bOverwriteExistingSave, bool& bSuccess);

	/* Deletes the game at SaveSlotName for player at UserIndex.
	* Calls LoadData internally if data hasn't been loaded yet.
	@param SaveSlotName			The name of the save slot to delete
	@param UserIndex			The index of the player to delete the game for
	*/
	UFUNCTION(BlueprintCallable, Category = "Expanded Save Game Library")
	static bool DeleteGame(const FString SaveSlotName, const int UserIndex);
	
	/* Renames the existing save slot under CurrentSlotName to NewSlotName
	@param CurrentSlotName		The name of the existing slot to change
	@param UserIndex			The index of the player to rename the game for
	@param NewSlotName			The name to change CurrentSlotName to
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void RenameSaveSlot(UObject* Context, const FString SaveSlotName, const int UserIndex, const FString NewSlotName, bool& bSuccess);

	/* This function receives any type of value and stores that data by the name of the pin connected to it

	This function is useful for each of the following situations:
	* You don't need to specify a name for the stored value
	* You need to quickly store a primitive value, like an integer
	* You need to store an array
	
	NOTE: Storing structs is supported, but getting the struct directly is currently not supported
	Struct values are stored under StructName + ContextName + ValueName, where Context is the object this function is called from

	Example Name: MyStructTopDownCharacter_C_0MyStructValue
	Example Usage: GetStoredInt("MyStructTopDownCharacter_C_0MyStructValue")
	@param Value				The value to store by the name of its pin
	*/
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "Value"), Category = "Expanded Save Game Library")
	static void StoreValue(UStruct* Value);

	/* Custom thunk for the StoreValue function that runs ProcessProperty on the last
	* property on the stack provided by StoreValue
	*/
	DECLARE_FUNCTION(execStoreValue)
	{
		//step into the stack to retrieve the values on it
		Stack.Step(Stack.Object, nullptr);

		//get the name of the UObject this function was called from
		//it will be used to store the data to connect the data to that object
		const FString& Name = Stack.Object->GetName();
		//the most recent property is the passed in by Value in StoreValue
		//this gives us the information provided by the input pin
		FProperty* StructProperty = Stack.MostRecentProperty;
		//we need the memory address that the value from the input pin is stored at
		void* StructPtr = Stack.MostRecentPropertyAddress;

		//mark that we're finished using the stack
		P_FINISH;

		//use the provided property information to get the values at the memory address
		ParseProperty(Name, StructProperty, StructPtr);
	}

	/* This function determines what type of value is being received by StoreValue,
	* then saves that value based on the name of the pin received by StoreValue.
	* @param Property		The wildcard value received by StoreValue or read by ProcessProperty
	* @param ValuePtr		The memory address of that value
	*/
	static void ParseProperty(const FString& ObjectName, FProperty* Property, void* ValuePtr, FString&& PropertyName = {});

	/* Detect property type and store it in the correct map under the current save */
	static bool StoreProperty(FProperty* Property, void* ValuePtr, const FString& PropertyName);

	/////////////////////////////////////////////////////////////////////////////////
	//	Everything below this line is a work in progress and not currently used
	/////////////////////////////////////////////////////////////////////////////////

#if 0
	/* Retrieve the struct value stored under StructName + ContextName + ValueName, where Context is the object this function is called from
	Example: MyStructTopDownCharacter_C_0MyStructValue

	WARNING: This function can be slow because it checks all stored containers for each variable in StructVariables
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Context", DefaultToSelf = "Context"), Category = "Expanded Save Game Library")
	static void GetStoredStruct(UObject* Context, const FString StructName, const TArray<FString> StructVariables, UStruct*& StoredStruct);
#endif
};