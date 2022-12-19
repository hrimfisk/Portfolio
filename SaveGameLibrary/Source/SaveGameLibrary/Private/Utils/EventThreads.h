// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameLibraryThread.h: Wrapper class to abstract the threading process.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"

#include "EngineUtils.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

class UWorld;

#define DECLARE_THREAD_CLASS(name)                      \
class name : public FRunnable                           \
{                                                       \
private:                                                \
    FRunnableThread* Thread;                            \
    const TArray<AActor*>& ActorsWithInterface;         \
                                                        \
public:                                                 \
    /*constructor*/                                     \
    name(const TArray<AActor*>& InActorsWithInterface); \
    ~name();                                            \
    uint32 Run() override;                              \
};                                                      \

DECLARE_THREAD_CLASS(PreSaveThread)
DECLARE_THREAD_CLASS(PostSaveThread)
DECLARE_THREAD_CLASS(PreLoadThread)
DECLARE_THREAD_CLASS(PostLoadThread)
DECLARE_THREAD_CLASS(NewSaveThread)