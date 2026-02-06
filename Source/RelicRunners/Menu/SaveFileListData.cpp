// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFileListData.h"
#include "ModeSelectionWidget.h"

FSaveData USaveFileListData::MakeSaveData(const FString& Name, float TimePlayed, int MetaLevel, FGuid UniqueID)
{
    FSaveData Data;
    Data.SavefileName = Name;
    Data.TimePlayed = TimePlayed;
    Data.MetaLevel = MetaLevel;
    Data.UniqueID = UniqueID;
    return Data;
}
