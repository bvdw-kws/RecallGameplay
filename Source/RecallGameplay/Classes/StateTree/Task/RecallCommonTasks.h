// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeTaskBase.h"
#include "Mass/EntityHandle.h"
#include "StateTreePropertyRef.h"
#include "System/AI/RecallStateTreeInstanceTypes.h"

#include "RecallCommonTasks.generated.h"

USTRUCT()
struct RECALLGAMEPLAY_API FRecallDelayTaskInstanceData
{
	GENERATED_BODY()

	/** Delay before the task ends. */
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(EditCondition="!bRunForever", Units="Seconds", ClampMin="0.0"))
	float Duration = 1.f;

	/** Adds random range to the Duration. */
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(EditCondition="!bRunForever", Units="Seconds", ClampMin="0.0"))
	float RandomDeviation = 0.f;

	/** If true the task will run forever until a transition stops it. */
	UPROPERTY(EditAnywhere, Category=Parameter)
	bool bRunForever = false;

	/** Internal countdown in seconds. */
	UPROPERTY()
	float RemainingTime = 0.f;
};

/**
 * Simple task to wait indefinitely or for a given time (in seconds) before succeeding.
 */
USTRUCT(meta=(DisplayName="Delay Task"))
struct RECALLGAMEPLAY_API FRecallDelayTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallDelayTaskInstanceData;

	FRecallDelayTask();
	
public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	virtual FName GetIconName() const override
	{
		return FName("StateTreeEditorStyle|Node.Time");
	}
	virtual FColor GetIconColor() const override
	{
		return UE::StateTree::Colors::Grey;
	}
#endif // WITH_EDITOR
};

UENUM()
enum class ERecallSendEventPayloadType : uint8
{
	Struct,
	Vector,
	Entity,
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallSendEventTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Parameter)
	TArray<FMassEntityHandle> TargetEntities;
	
	/** Tag describing the event */
	UPROPERTY(EditAnywhere, Category=Parameter, meta=(Categories="StateTreeEvent"))
	FGameplayTag Tag;

	/** Optional payload for the event. */
	UPROPERTY(EditAnywhere, Category=Parameter)
	ERecallSendEventPayloadType PayloadType = ERecallSendEventPayloadType::Struct;

	UPROPERTY(EditAnywhere, Category=Parameter, meta=(EditCondition="PayloadType == ERecallSendEventPayloadType::Struct", EditConditionHides), DisplayName="Payload")
	FInstancedStruct StructPayload;

	UPROPERTY(EditAnywhere, Category=Parameter, meta=(EditCondition="PayloadType == ERecallSendEventPayloadType::Vector", EditConditionHides), DisplayName="Payload")
	FVector VectorPayload = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category=Parameter, meta=(EditCondition="PayloadType == ERecallSendEventPayloadType::Entity", EditConditionHides), DisplayName="Payload")
	FMassEntityHandle EntityPayload;

	/** Optional info to describe who sent the event. */
	UPROPERTY(EditAnywhere, Category=Parameter)
	FName Origin;
};

/**
 * Simple task to wait indefinitely or for a given time (in seconds) before succeeding.
 */
USTRUCT(meta=(DisplayName="Send Event"))
struct RECALLGAMEPLAY_API FRecallSendEventTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallSendEventTaskInstanceData;

public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	UPROPERTY(EditAnywhere, Category=Parameters)
	bool bSucceedOnSent = true;
	
	UPROPERTY(EditAnywhere, Category=Parameter, AdvancedDisplay)
	TArray<FName> SubStateTreeNames;
	
	UPROPERTY(EditAnywhere, Category=Parameter, AdvancedDisplay)
	bool bSendEventToRootStateTree = true;
	
private:
	TStateTreeExternalDataHandle<class URecallStateTreeSubsystem> StateTreeSystemHandle;

	bool SendEvent(FStateTreeExecutionContext& Context) const;
	FInstancedStruct GetPayload(FStateTreeExecutionContext& Context) const;

	TArray<FRecallStateTreeInstanceHandle> GetStateTreeHandlesFromEntities(FStateTreeExecutionContext& Context, const TArray<FMassEntityHandle>& Entities) const;
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallDebugPrintTaskInstanceData
{
	GENERATED_BODY()
};

USTRUCT(meta=(DisplayName="Debug Print"))
struct RECALLGAMEPLAY_API FRecallDebugPrintTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallDebugPrintTaskInstanceData;

public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	UPROPERTY(EditAnywhere)
	FString MessageOnEnter;
	
	UPROPERTY(EditAnywhere)
	FString MessageOnTick;
	
	UPROPERTY(EditAnywhere)
	FString MessageOnExit;

	UPROPERTY(EditAnywhere)
	int32 Key = -1;

	UPROPERTY(EditAnywhere)
	float TimeToDisplay = 4.0f;

	UPROPERTY(EditAnywhere)
	FColor DisplayColor = FColor::Green;

private:
	void Print(FStateTreeExecutionContext& Context, const FString& DebugMessage) const;
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallStateTreeTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category=Output)
	FRecallStateTreeInstanceHandle InstanceHandle;
};

USTRUCT(meta=(DisplayName="State Tree"))
struct RECALLGAMEPLAY_API FRecallStateTreeTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallStateTreeTaskInstanceData;

public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStateTree> StateTree;

	UPROPERTY(EditAnywhere)
	FName StateTreeName = NAME_None;

private:
	TStateTreeExternalDataHandle<struct FRecallStateTreeInstanceFragment> StateTreeInstanceFragmentHandle;
	TStateTreeExternalDataHandle<class URecallStateTreeSubsystem> StateTreeSystemHandle;
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallDestroyTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Parameter)
	TArray<FMassEntityHandle> Entities;
};

USTRUCT(meta=(DisplayName="Destroy Entity"))
struct RECALLGAMEPLAY_API FRecallDestroyTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallDestroyTaskInstanceData;

public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
