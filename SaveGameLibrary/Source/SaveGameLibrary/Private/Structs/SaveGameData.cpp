// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SaveGameData.h: The data that save game slots manage. Slots and data are
    both mapped using the save slot name, ensuring that the Slot and Data entries
    match up.
=============================================================================*/

#include "SaveGameData.h"

//initialize member variables
FSaveGameData::FSaveGameData()
    : Slots{}
    , Data{}
{
}