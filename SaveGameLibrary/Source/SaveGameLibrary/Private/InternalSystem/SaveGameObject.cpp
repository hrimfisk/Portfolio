// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameObject.h: Subclass of USaveGame for saving data to a slot.
=============================================================================*/

#include "SaveGameObject.h"

USaveGameObject::USaveGameObject()
    : Slots{}
    , Data{}
{
}

void USaveGameObject::SetData(const FSaveGameData& InData)
{
    Slots = InData.Slots;
    Data = InData.Data;
}
