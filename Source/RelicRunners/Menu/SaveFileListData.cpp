// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveFileListData.h"
#include "ModeSelectionWidget.h"

void USaveFileListData::AddMetaExp(int Value)
{
    SaveData.MetaExperience += Value;
    while (SaveData.MetaExperience >= SaveData.MetaExperienceToLevel && SaveData.MetaLevel > 0)
    {
        SaveData.MetaExperience -= SaveData.MetaExperienceToLevel;
        SaveData.MetaExperienceToLevel *= 2;
        SaveData.MetaPoints++;
        SaveData.MetaLevel++;
    }
}

FSaveData USaveFileListData::MakeSaveData(const FString& Name, float TimePlayed, int MetaLevel, FGuid UniqueID, float MetaExperience,
    float MetaExperienceToLevel, int MetaPoints, int MetaHealth, int MetaDefence, int MetaLuck, int MetaStrength, int MetaAgility, 
    int MetaIntelligence, int MetaStartingLevel, int MetaPotionEfficiency, int MetaResourceRegen )
{
    FSaveData Data;
    Data.SavefileName = Name;
    Data.TimePlayed = TimePlayed;
    Data.MetaLevel = MetaLevel;
    Data.UniqueID = UniqueID;
    Data.MetaExperience = MetaExperience;
    Data.MetaExperienceToLevel = MetaExperienceToLevel;
    Data.MetaPoints = MetaPoints;
    Data.MetaHealth = MetaHealth;
    Data.MetaDefence = MetaDefence;
    Data.MetaLuck = MetaLuck;
    Data.MetaStrength = MetaStrength;
    Data.MetaAgility = MetaAgility;
    Data.MetaIntelligence = MetaIntelligence;
    Data.MetaStartingLevel = MetaStartingLevel;
    Data.MetaPotionEfficiency = MetaPotionEfficiency;
    Data.MetaResourceRegen = MetaResourceRegen;

    return Data;
}
