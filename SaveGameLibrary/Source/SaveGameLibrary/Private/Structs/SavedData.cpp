// Copyright Thomas Howard Jensen published 2022

/*=============================================================================
    SavedData.h: These maps get added to with StoreValue functions from the 
    ExpandedSaveGameLibrary and retrieve from when using GetStoredValue functions.
=============================================================================*/

#include "SavedData.h"

FSavedData::FSavedData()
    : Version{ 1.2f }
    , ActorNames{}
    , Classes{}
    , Transforms{}
    , Vectors{}
    , Rotators{}
    , Strings{}
    , Texts{}
    , Names{}
    , Integers{}
    , Floats{}
    , Booleans{}
    , SecondsPlayed{ 0 }
    , ProjectVersion{}
{}