// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
	SaveGameEvents.h: The interface that blueprints can use to implement
	additional features.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveGameEvents.generated.h"

//do this on a separate thread
#define CALL_INTERFACE_FUNCTION_ON_ALL_ACTORS(actors, func)                         \
for (const auto& Actor : actors)													\
{                                                                                   \
    /*check for blueprint interface*/                                               \
    if (ISaveGameEvents* Interface = Cast<ISaveGameEvents>(Actor))					\
    {                                                                               \
        Interface->func(Actor);														\
    }                                                                               \
    /*check for C++ interface*/                                                     \
    else if (Actor->GetClass()->ImplementsInterface(USaveGameEvents::StaticClass()))\
    {                                                                               \
        ISaveGameEvents::func(Actor);												\
    }                                                                               \
}

UINTERFACE(Blueprintable, MinimalAPI)
class USaveGameEvents : public UInterface
{
	GENERATED_BODY()
};

class ISaveGameEvents
{
	GENERATED_BODY()

public:
	/* This event runs when SaveGame is called before the file is written to 
	NOTE: If multithreading is enabled, this event will run, but it will not trigger a breakpoint */
	UFUNCTION(BlueprintNativeEvent, Category = "Save Game Events")
	void PreSave();
	/* This event runs when SaveGame is called after the file is written to 
	NOTE: If multithreading is enabled, this event will run, but it will not trigger a breakpoint */
	UFUNCTION(BlueprintNativeEvent, Category = "Save Game Events")
	void PostSave();
	/* This event runs when LoadGame is called before the data is retrieved. If the data has not been loaded yet using LoadData, this will be called before LoadData is called.
	NOTE: If multithreading is enabled, this event will run, but it will not trigger a breakpoint */
	UFUNCTION(BlueprintNativeEvent, Category = "Save Game Events")
	void PreLoad();
	/* This event runs when LoadGame is called after the data is retrieved. It's useful for setting data on actors as soon as their data is ready. 
	WARNING: This event is does not currently support multithreading, and calling SpawnStoredActors from this event with Multithreaded Events enabled will halt execution of the program */
	UFUNCTION(BlueprintNativeEvent, Category = "Save Game Events")
	void PostLoad();
	/* This event runs when NewSaveGame is created to store any values that need to be saved for this actor. 
	NOTE: If multithreading is enabled, this event will run, but it will not trigger a breakpoint */
	UFUNCTION(BlueprintNativeEvent, Category = "Save Game Events")
	void NewSaveCreated();
};
