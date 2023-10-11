// Fill out your copyright notice in the Description page of Project Settings.


#include "AGPGameInstance.h"


UClass* UAGPGameInstance::GetWeaponPickupClass() const
{
	return WeaponPickupClass.Get();
}

UClass* UAGPGameInstance::GetLevelBoxClass() const
{
	
}

UClass* UAGPGameInstance::GetRoomClass() const
{
	return RoomClass.Get();
}
