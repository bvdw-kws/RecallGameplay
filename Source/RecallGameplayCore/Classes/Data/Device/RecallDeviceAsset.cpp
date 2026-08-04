// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallDeviceAsset.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(Device_Dummy,	"Device.Dummy",		"Dummy device");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Device_Parent,	"Device",			"Device parent");

const FPrimaryAssetType URecallDeviceAsset::AssetType("MSDeviceData");

FPrimaryAssetId URecallDeviceAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(URecallDeviceAsset::AssetType, GetFName());
}
