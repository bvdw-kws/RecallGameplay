#include "System/Device/RecallDevicePreloadSubsystem.h"

#include "Data/Device/RecallDeviceAsset.h"
#include "System/Device/RecallDeviceSubsystem.h"
#include "System/Asset/RecallAssetManagerSubsystem.h"

void URecallDevicePreloadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	AssetManagerSystem = UWorld::GetSubsystem<URecallAssetManagerSubsystem>(GetWorld());
}

void URecallDevicePreloadSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	AssetManagerSystem.Reset();
}

void URecallDevicePreloadSubsystem::Start(const FRecallSimulationStartParams& Params)
{
	PreloadDeviceEntityConfigs();
}

void URecallDevicePreloadSubsystem::Reset()
{
	PreloadedEntityConfigHandles.Reset();
}

void URecallDevicePreloadSubsystem::Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot)
{
}

void URecallDevicePreloadSubsystem::Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot)
{
}

void URecallDevicePreloadSubsystem::PreloadDeviceEntityConfigs()
{
	check(PreloadedEntityConfigHandles.IsEmpty());
	
	if (!ensureMsgf(AssetManagerSystem.IsValid(),
		TEXT("%hs - AssetManagerSystem not found"), __FUNCTION__))
	{
		return;
	}

	const URecallDeviceSubsystem& DeviceSubsystem = URecallDeviceSubsystem::GetRef(GetWorld());
	
	int32 TotalPreloadedCount = 0;
	int32 TotalSkippedCount = 0;

	// Get all device assets from the device subsystem
	for (const TPair<FName, TObjectPtr<URecallDeviceAsset>>& DeviceAssetPair : DeviceSubsystem.GetAllDeviceAssets())
	{
		const URecallDeviceAsset* DeviceAsset = DeviceAssetPair.Value;
		if (!DeviceAsset || DeviceAsset->EntityConfig.IsNull())
		{
			continue;
		}

		if (!DeviceAsset->bPreloadEntityConfig)
		{
			TotalSkippedCount++;
			continue;
		}

		const FRecallAssetLoadHandle LoadHandle = AssetManagerSystem->RequestAsset(DeviceAsset->EntityConfig, 5);
		if (LoadHandle.IsValid())
		{
			PreloadedEntityConfigHandles.Add(DeviceAsset->DeviceTag.GetTagName(), LoadHandle);
			TotalPreloadedCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%hs - Preloaded %d device entity configs, skipped %d devices"), 
		__FUNCTION__, TotalPreloadedCount, TotalSkippedCount);
}
