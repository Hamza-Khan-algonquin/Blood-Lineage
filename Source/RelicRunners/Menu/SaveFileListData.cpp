// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFileListData.h"
#include "ModeSelectionWidget.h"

FSaveData USaveFileListData::MakeSaveData(const FString& Name, float TimePlayed, int NumUpgrades)
{
    FSaveData Data;
    Data.SavefileName = Name;
    Data.TimePlayed = TimePlayed;
    Data.NumUpgrades = NumUpgrades;
    return Data;
}
