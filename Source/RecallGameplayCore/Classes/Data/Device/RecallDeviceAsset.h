// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/DataAsset.h"
#include "NativeGameplayTags.h"
#include "StructUtils/InstancedStruct.h"

#include "RecallDeviceAsset.generated.h"

RECALLGAMEPLAYCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Device_Dummy);
RECALLGAMEPLAYCORE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Device_Parent);

UCLASS()
class RECALLGAMEPLAYCORE_API URecallDeviceAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Tag used to reference this device.
	 */
	UPROPERTY(EditAnywhere, meta=(GameplayTagFilter="Device"))
	FGameplayTag DeviceTag;

	/**
	 * Icon of the device.
	 */
	UPROPERTY(EditAnywhere, meta=( AllowPrivateAccess="true", DisplayThumbnail="true", DisplayName="Image", AllowedClasses="/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"))
	TSoftObjectPtr<UObject> Icon;

	/**
	 * Localized name of the device.
	 */
	UPROPERTY(EditAnywhere)
	FText DeviceName;

	/**
	 * Localized description of the device.
	 */
	UPROPERTY(EditAnywhere)
	FText DeviceDescription;
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="/Script/RecallDeviceModule.RecallDeviceCostBase", ExcludeBaseStruct))
	FInstancedStruct Cost;
	
	/**
	 * Entity config of the device to place.
	 */
	UPROPERTY(EditAnywhere, meta=(AllowedClasses="/Script/MassSpawner.MassEntityConfigAsset"))
	FSoftObjectPath EntityConfig;

	/**
	 * Whether to preload this device's entity config when simulation starts.
	 * This improves performance by avoiding loading delays during device spawning.
	 */
	UPROPERTY(EditAnywhere, Category="Performance")
	bool bPreloadEntityConfig = true;

public:
	static const FPrimaryAssetType AssetType;

protected:	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
};
