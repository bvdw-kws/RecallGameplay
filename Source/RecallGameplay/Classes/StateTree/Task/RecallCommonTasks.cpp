// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallCommonTasks.h"

#include "Desync/RecallDesyncLog.h"
#include "MassCommands.h"
#include "MassEntityManager.h"
#include "MassEntityView.h"
#include "RecallSignalSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "StateTree/RecallStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Simulation/Destroy/RecallDestroySignalTypes.h"
#include "Simulation/StateTree/RecallStateTreeFragments.h"
#include "Simulation/StateTree/RecallStateTreeSignalTypes.h"
#include "System/AI/RecallStateTreeSubsystem.h"

#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
#include "Engine/Engine.h"
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

#define LOCTEXT_NAMESPACE "RecallCommonTasks"

//----------------------------------------------------------------------//
// FRecallDelayTask
//----------------------------------------------------------------------//
FRecallDelayTask::FRecallDelayTask()
{
	bConsideredForScheduling = false;
	bShouldCopyBoundPropertiesOnTick = false;
	bShouldCopyBoundPropertiesOnExitState = false;
}

bool FRecallDelayTask::Link(FStateTreeLinker& Linker)
{
	return true;
}

EStateTreeRunStatus FRecallDelayTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.bRunForever)
	{
		const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
		
		const FRandomStream& RandomStream = MassContext.GetRandomStream();
		InstanceData.RemainingTime = RandomStream.FRandRange(
			FMath::Max(0.0f, InstanceData.Duration - InstanceData.RandomDeviation), (InstanceData.Duration + InstanceData.RandomDeviation));

		MassContext.GetSignalSystem().DelaySignalEntity(
			Recall::StateTree::Signals::TickRequired, MassContext.GetEntity(), InstanceData.RemainingTime);

#if RECALL_DESYNC_LOG
		RECALL_DESYNC_LOG_FLOAT(Context.GetWorld(), DelayTaskRemainingTime, InstanceData.RemainingTime);
#endif // RECALL_DESYNC_LOG
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FRecallDelayTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.bRunForever)
	{
		InstanceData.RemainingTime -= DeltaTime;

		if (InstanceData.RemainingTime <= UE_KINDA_SMALL_NUMBER)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FRecallDelayTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	check(InstanceData);

	FText Value = FText::GetEmpty();

	if (const FPropertyBindingPath* RunForeverSourcePath = BindingLookup.GetPropertyBindingSource(
			FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FInstanceDataType, bRunForever))))
	{
		Value = FText::Format(LOCTEXT("ForeverBound", "Forever={0}"),
			BindingLookup.GetPropertyPathDisplayName(*RunForeverSourcePath, Formatting));
	}
	else if (InstanceData->bRunForever)
	{
		Value = LOCTEXT("Forever", "Forever");
	}
	else
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 1;
		Options.MaximumFractionalDigits = 3;

		FText DurationText = BindingLookup.GetBindingSourceDisplayName(
			FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FInstanceDataType, Duration)), Formatting);
		if (DurationText.IsEmpty())
		{
			DurationText = FText::AsNumber(InstanceData->Duration, &Options);
		}

		FText RandomDeviationText = BindingLookup.GetBindingSourceDisplayName(FPropertyBindingPath(ID, GET_MEMBER_NAME_CHECKED(FInstanceDataType, RandomDeviation)), Formatting);
		if (RandomDeviationText.IsEmpty()
			&& !FMath::IsNearlyZero(InstanceData->RandomDeviation))
		{
			RandomDeviationText = FText::AsNumber(InstanceData->RandomDeviation, &Options);
		}

		if (RandomDeviationText.IsEmpty())
		{
			Value = DurationText;
		}
		else
		{
			if (Formatting == EStateTreeNodeFormatting::RichText)
			{
				Value = FText::Format(LOCTEXT("DelayValueRich", "{0} <s>\u00B1{1}</>"), // +-
					DurationText,
					RandomDeviationText);
			}
			else
			{
				Value = FText::Format(LOCTEXT("DelayValue", "{0} \u00B1{1}"), // +-
					DurationText,
					RandomDeviationText);
			}
		}
	}

	const FText Format = (Formatting == EStateTreeNodeFormatting::RichText)
		? LOCTEXT("DelayRich", "<b>Delay</> {Time}")
		: LOCTEXT("Delay", "Delay {Time}");

	return FText::FormatNamed(Format,
		TEXT("Time"), Value);
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

//----------------------------------------------------------------------//
// FRecallSendEventTask
//----------------------------------------------------------------------//
bool FRecallSendEventTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(StateTreeSystemHandle);
	return true;
}

TArray<FRecallStateTreeInstanceHandle> FRecallSendEventTask::GetStateTreeHandlesFromEntities(FStateTreeExecutionContext& Context, const TArray<FMassEntityHandle>& Entities) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	TArray<FRecallStateTreeInstanceHandle> Results;

	FRecallStateTreeExecutionContext& RecallContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = RecallContext.GetEntityManager();

	for (const FMassEntityHandle& Entity : Entities)
	{
		if (!EntityManager.IsEntityValid(Entity))
		{
			continue;
		}

		const FMassEntityView EntityView(EntityManager, Entity);
		const FRecallStateTreeInstanceFragment* StateTreeInstanceFragmentPtr = EntityView.GetFragmentDataPtr<FRecallStateTreeInstanceFragment>();
		if (StateTreeInstanceFragmentPtr == nullptr)
		{
			continue;
		}

		if (bSendEventToRootStateTree)
		{
			Results.Add(StateTreeInstanceFragmentPtr->InstanceHandle);
		}

		if (SubStateTreeNames.IsEmpty())
		{
			Results.Append(StateTreeInstanceFragmentPtr->SubInstanceHandles);
		}
		else
		{
			for (const FName& SubStateTreeName : SubStateTreeNames)
			{
				const FRecallStateTreeInstanceHandle* InstanceHandle = StateTreeInstanceFragmentPtr->SubInstanceHandleMap.Find(SubStateTreeName);
				if (InstanceHandle != nullptr)
				{
					Results.Add(*InstanceHandle);
				}
				else
				{
					UE_LOG(LogMass, Log, TEXT("Could not find running sub state tree named: %s"), *SubStateTreeName.ToString());
				}
			}
		}
	}

	return Results;
}

FInstancedStruct FRecallSendEventTask::GetPayload(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	switch (InstanceData.PayloadType)
	{
	case ERecallSendEventPayloadType::Entity:
		return FInstancedStruct::Make<FMassEntityHandle>(InstanceData.EntityPayload);

	case ERecallSendEventPayloadType::Vector:
		return FInstancedStruct::Make<FVector>(InstanceData.VectorPayload);

	case ERecallSendEventPayloadType::Struct:
		return InstanceData.StructPayload;

	default:
		unimplemented();
		return FInstancedStruct();
	}
}

bool FRecallSendEventTask::SendEvent(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const TArray<FRecallStateTreeInstanceHandle> InstanceHandles = GetStateTreeHandlesFromEntities(Context, InstanceData.TargetEntities);

	if (InstanceHandles.Num() == 0)
	{
		return false;
	}

	const FRecallStateTreeExecutionContext& RecallContext = static_cast<FRecallStateTreeExecutionContext&>(Context);

	const FStateTreeEvent Event(InstanceData.Tag, GetPayload(Context), InstanceData.Origin);
	URecallStateTreeSubsystem& StateTreeSystem = Context.GetExternalData(StateTreeSystemHandle);

	for (const FRecallStateTreeInstanceHandle& InstanceHandle : InstanceHandles)
	{
		StateTreeSystem.SendStateTreeEvent(InstanceHandle, Event);

#if RECALL_DESYNC_LOG
		RECALL_DESYNC_LOG_CONTEXT(Context.GetWorld(), FString::Printf(TEXT("%s send event <%s> to StateTreeInstance<%s>"), *RecallContext.GetEntity().DebugGetDescription(), *InstanceData.Tag.ToString(), *InstanceHandle.DebugGetDescription()));
#endif // RECALL_DESYNC_LOG
	}

	// Wake up state tree of target entities
	RecallContext.GetSignalSystem().SignalEntities(Recall::StateTree::Signals::EventReceived, InstanceData.TargetEntities);

	return true;
}

EStateTreeRunStatus FRecallSendEventTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (SendEvent(Context))
	{
		if (bSucceedOnSent)
		{
			return EStateTreeRunStatus::Succeeded;
		}
		return Super::EnterState(Context, Transition);
	}
	else
	{
		return EStateTreeRunStatus::Failed;
	}
}

//----------------------------------------------------------------------//
// FRecallDebugPrintTask
//----------------------------------------------------------------------//
bool FRecallDebugPrintTask::Link(FStateTreeLinker& Linker)
{
	return true;
}

void FRecallDebugPrintTask::Print(FStateTreeExecutionContext& Context, const FString& DebugMessage) const
{
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	if (!DebugMessage.IsEmpty() && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, TimeToDisplay, DisplayColor, DebugMessage);
	}
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
}

EStateTreeRunStatus FRecallDebugPrintTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Print(Context, MessageOnEnter);
	return Super::EnterState(Context, Transition);
}

EStateTreeRunStatus FRecallDebugPrintTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	Print(Context, MessageOnTick);
	return Super::Tick(Context, DeltaTime);
}

void FRecallDebugPrintTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	Print(Context, MessageOnExit);
	return Super::ExitState(Context, Transition);
}

//----------------------------------------------------------------------//
// FRecallStateTreeTask
//----------------------------------------------------------------------//
bool FRecallStateTreeTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(StateTreeInstanceFragmentHandle);
	Linker.LinkExternalData(StateTreeSystemHandle);
	return true;
}

EStateTreeRunStatus FRecallStateTreeTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (!ensureMsgf(StateTree, TEXT("State tree must be set")))
	{
		return EStateTreeRunStatus::Failed;
	}

	URecallStateTreeSubsystem& StateTreeSystem = Context.GetExternalData(StateTreeSystemHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!ensureMsgf(!InstanceData.InstanceHandle.IsValid(), TEXT("State tree is already running (somehow)")))
	{
		StateTreeSystem.FreeInstanceData(InstanceData.InstanceHandle);
	}

	const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	FRecallStateTreeInstanceFragment& StateTreeInstanceFragment = Context.GetExternalData(StateTreeInstanceFragmentHandle);

	const int32 RandomSeed = MassContext.GetRandomStream().RandRange(TNumericLimits<int32>::Min(), TNumericLimits<int32>::Max());

	InstanceData.InstanceHandle = StateTreeSystem.AllocateInstanceData();
	if (!ensureMsgf(InstanceData.InstanceHandle.IsValid(), TEXT("Failed to create state tree instance data")))
	{
		return EStateTreeRunStatus::Failed;
	}

	StateTreeInstanceFragment.SubInstanceHandles.Add(InstanceData.InstanceHandle);

	if (!StateTreeName.IsNone() && ensureMsgf(!StateTreeInstanceFragment.SubInstanceHandleMap.Contains(StateTreeName), TEXT("Already a state tree named: %s"), *StateTreeName.ToString()))
	{
		StateTreeInstanceFragment.SubInstanceHandleMap.Add(StateTreeName, InstanceData.InstanceHandle);
	}

	URecallSignalSubsystem& SignalSystem = MassContext.GetSignalSystem();
	FStateTreeInstanceData* StateTreeInstanceDataPtr = StateTreeSystem.GetInstanceData(InstanceData.InstanceHandle);

	FRecallStateTreeExecutionContext StateTreeContext(StateTreeSystem, *StateTree, *StateTreeInstanceDataPtr, MassContext.GetEntityManager(), SignalSystem, MassContext.GetMassExecutionContext(), MassContext.GetEntity());
	StateTreeContext.Start(FStateTreeExecutionContext::FStartParameters{ .RandomSeed = RandomSeed });

	return Super::EnterState(Context, Transition);
}

EStateTreeRunStatus FRecallStateTreeTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	URecallStateTreeSubsystem& StateTreeSystem = Context.GetExternalData(StateTreeSystemHandle);
	URecallSignalSubsystem& SignalSystem = MassContext.GetSignalSystem();
	FStateTreeInstanceData* StateTreeInstanceDataPtr = StateTreeSystem.GetInstanceData(InstanceData.InstanceHandle);

	FRecallStateTreeExecutionContext StateTreeContext(StateTreeSystem, *StateTree, *StateTreeInstanceDataPtr, MassContext.GetEntityManager(), SignalSystem, MassContext.GetMassExecutionContext(), MassContext.GetEntity());

	// Tick the tree instance
	StateTreeContext.Tick(DeltaTime);

	// When last tick status is different than "Running", the state tree need to be tick again
	// For performance reason, tick again to see if we could find a new state right away instead of waiting the next frame.
	if (StateTreeContext.GetLastTickStatus() != EStateTreeRunStatus::Running)
	{
		StateTreeContext.Tick(0.0f);
	}

	if (StateTreeContext.GetLastTickStatus() == EStateTreeRunStatus::Unset)
	{
		return Super::Tick(Context, DeltaTime);
	}

	return StateTreeContext.GetLastTickStatus();
}

void FRecallStateTreeTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	URecallStateTreeSubsystem& StateTreeSystem = Context.GetExternalData(StateTreeSystemHandle);
	FRecallStateTreeInstanceFragment& StateTreeInstanceFragment = Context.GetExternalData(StateTreeInstanceFragmentHandle);

	URecallSignalSubsystem& SignalSystem = MassContext.GetSignalSystem();
	FStateTreeInstanceData* StateTreeInstanceDataPtr = StateTreeSystem.GetInstanceData(InstanceData.InstanceHandle);

	FRecallStateTreeExecutionContext StateTreeContext(StateTreeSystem, *StateTree, *StateTreeInstanceDataPtr, MassContext.GetEntityManager(), SignalSystem, MassContext.GetMassExecutionContext(), MassContext.GetEntity());
	StateTreeContext.Stop();

	if (!StateTreeName.IsNone() && ensureMsgf(StateTreeInstanceFragment.SubInstanceHandleMap.Contains(StateTreeName), TEXT("Named state tree was already removed: %s"), *StateTreeName.ToString()))
	{
		StateTreeInstanceFragment.SubInstanceHandleMap.Remove(StateTreeName);
	}

	StateTreeInstanceFragment.SubInstanceHandles.Remove(InstanceData.InstanceHandle);
	StateTreeSystem.FreeInstanceData(InstanceData.InstanceHandle);

	return Super::ExitState(Context, Transition);
}

//----------------------------------------------------------------------//
// FRecallDestroyTask
//----------------------------------------------------------------------//
bool FRecallDestroyTask::Link(FStateTreeLinker& Linker)
{
	return true;
}

EStateTreeRunStatus FRecallDestroyTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FRecallStateTreeExecutionContext& MassContext = static_cast<FRecallStateTreeExecutionContext&>(Context);
	URecallSignalSubsystem& SignalSystem = MassContext.GetSignalSystem();

	if (InstanceData.Entities.Num())
	{
		SignalSystem.SignalEntities(Recall::Destroy::Signals::Destroy, InstanceData.Entities);
	}

	return Super::EnterState(Context, Transition);
}
