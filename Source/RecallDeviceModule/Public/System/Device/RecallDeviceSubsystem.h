// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/SoftObjectPtr.h"

#include "RecallDeviceSubsystem.generated.h"

class URecallDeviceAsset;

UCLASS()
class RECALLDEVICEMODULE_API URecallDeviceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static const URecallDeviceSubsystem& GetRef(const UWorld* World);

public:
	TObjectPtr<URecallDeviceAsset> GetDeviceAsset(const FGameplayTag& DeviceTag) const;
	TObjectPtr<URecallDeviceAsset> GetDeviceAssetByTagName(const FName& DeviceTagName) const;
	TArray<TObjectPtr<URecallDeviceAsset>> GetDeviceAssetsByParent(const FGameplayTag& DeviceParentTag) const;
	
	// Get all device assets (for preloading subsystem)
	const TMap<FName, TObjectPtr<URecallDeviceAsset>>& GetAllDeviceAssets() const { return DeviceAssetMap; }

protected:
	UFUNCTION(BlueprintPure, Category="Device", meta=(BlueprintThreadSafe, DisplayName="Get Device Asset"))
	TSoftObjectPtr<URecallDeviceAsset> BP_GetDeviceAsset(FGameplayTag DeviceTag) const;
	
	// USubsystem implementation Begin
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override final;
	virtual void Deinitialize() override final;
	// USubsystem implementation End

private:
	UPROPERTY(Transient)
	TArray<TSoftObjectPtr<URecallDeviceAsset>> DeviceAssetPtrs;
	UPROPERTY(Transient)
	TMap<FName, TSoftObjectPtr<URecallDeviceAsset>> DeviceAssetPtrMap;
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<URecallDeviceAsset>> DeviceAssetMap;
};
