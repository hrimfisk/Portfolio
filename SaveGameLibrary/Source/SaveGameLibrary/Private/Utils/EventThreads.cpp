// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameLibraryThread.h: Wrapper class to abstract the threading process.
=============================================================================*/

#include "EventThreads.h"
#include "Engine/World.h"
#include "../../Public/SaveGameObject.h"
#include "../InternalSystem/SaveGameEvents.h"
#include "GameFramework/Actor.h"

#define DEFINE_THREAD_CLASS(name, func)                                                     \
name::name(const TArray<AActor*>& InActorsWithInterface)                                    \
    : ActorsWithInterface{ InActorsWithInterface }                                          \
{                                                                                           \
    Thread = FRunnableThread::Create(this, TEXT(#name));                                    \
    Init();                                                                                 \
}                                                                                           \
                                                                                            \
name::~name()                                                                               \
{                                                                                           \
    Thread->Kill();                                                                         \
    delete Thread;                                                                          \
    Thread = nullptr;                                                                       \
}                                                                                           \
                                                                                            \
uint32 name::Run()                                                                          \
{                                                                                           \
    for (const auto& Actor : ActorsWithInterface)                                           \
    {                                                                                       \
        /*check for blueprint interface*/                                                   \
        if (ISaveGameEvents* Interface = Cast<ISaveGameEvents>(Actor))                      \
        {                                                                                   \
            Interface->func(Actor);                                                         \
        }                                                                                   \
        /*check for C++ interface*/                                                         \
        else if (Actor->GetClass()->ImplementsInterface(USaveGameEvents::StaticClass()))    \
        {                                                                                   \
            ISaveGameEvents::func(Actor);                                                   \
        }                                                                                   \
    }                                                                                       \
    return 0;                                                                               \
}                                                                                           \

DEFINE_THREAD_CLASS(PreSaveThread, Execute_PreSave)
DEFINE_THREAD_CLASS(PostSaveThread, Execute_PostSave)
DEFINE_THREAD_CLASS(PreLoadThread, Execute_PreLoad)
DEFINE_THREAD_CLASS(PostLoadThread, Execute_PostLoad)
DEFINE_THREAD_CLASS(NewSaveThread, Execute_NewSaveCreated)
