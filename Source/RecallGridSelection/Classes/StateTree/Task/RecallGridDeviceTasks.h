// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeTaskBase.h"
#include "GameplayTagContainer.h"

#include "RecallGridDeviceTasks.generated.h"

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridDestroyDeviceTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category=Parameter)
	int32 GridCellIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category=Output, meta=(GameplayTagFilter="Device"))
	FGameplayTag DestroyedDeviceTag;
};

USTRUCT(meta=(DisplayName="Destroy Device On Grid"))
struct RECALLGRIDSELECTION_API FRecallGridDestroyDeviceTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallGridDestroyDeviceTaskInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	TStateTreeExternalDataHandle<class URecallGridSelectionSubsystem> GridSelectionSystemHandle;
};

USTRUCT()
struct RECALLGRIDSELECTION_API FRecallGridSpawnDeviceTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category=Parameter)
	int32 GridCellIndex = INDEX_NONE;
	
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(GameplayTagFilter="Device"))
	FGameplayTag Device;
	
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(BaseStruct="/Script/RecallGridSelection.RecallGridDeviceSpawnCommand"))
	FInstancedStruct SpawnCommand;
};

USTRUCT(meta=(DisplayName="Spawn Device On Grid"))
struct RECALLGRIDSELECTION_API FRecallGridSpawnDeviceTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallGridSpawnDeviceTaskInstanceData;

protected:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	TStateTreeExternalDataHandle<class URecallGridSelectionSubsystem> GridSelectionSystemHandle;
	TStateTreeExternalDataHandle<class URecallEntityAsyncSpawnSubsystem> EntityAsyncSpawnSystemHandle;
};
