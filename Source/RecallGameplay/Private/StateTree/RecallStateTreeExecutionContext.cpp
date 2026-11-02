// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "StateTree/RecallStateTreeExecutionContext.h"

#include "MassEntityView.h"
#include "RecallSignalSubsystem.h"
#include "Simulation/StateTree/RecallStateTreeSignalTypes.h"

namespace UE::MassBehavior
{
bool SetContextData(FStateTreeExecutionContext& Context)
{
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(StateTreeProcessorSetContextData);

	const FRecallStateTreeExecutionContext& RecallContext = static_cast<const FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = RecallContext.GetEntityManager();

	bool bFoundAll = true;
	const FMassEntityView EntityView(EntityManager, RecallContext.GetEntity());

	for (const FStateTreeExternalDataDesc& DataDesc : Context.GetContextDataDescs())
	{
		if (DataDesc.Struct == nullptr)
		{
			continue;
		}

		if (DataDesc.Struct->IsChildOf(FMassFragment::StaticStruct()))
		{
			const UScriptStruct* ScriptStruct = Cast<const UScriptStruct>(DataDesc.Struct);
			FStructView Fragment = EntityView.GetFragmentDataStruct(ScriptStruct);
			if (Fragment.IsValid())
			{
				Context.SetContextData(DataDesc.Handle, FStateTreeDataView(Fragment));
			}
			else
			{
				bFoundAll = false;
			}
		}
		else if (DataDesc.Struct->IsChildOf(FMassSharedFragment::StaticStruct()))
		{
			const UScriptStruct* ScriptStruct = Cast<const UScriptStruct>(DataDesc.Struct);
			FStructView SharedFragment = EntityView.GetSharedFragmentDataStruct(ScriptStruct);
			if (SharedFragment.IsValid())
			{
				Context.SetContextData(DataDesc.Handle, FStateTreeDataView(SharedFragment));
			}
			else
			{
				bFoundAll = false;
			}
		}
		else if (DataDesc.Struct->IsChildOf(FMassConstSharedFragment::StaticStruct()))
		{
			const UScriptStruct* ScriptStruct = Cast<const UScriptStruct>(DataDesc.Struct);
			FConstStructView ConstSharedFragment = EntityView.GetConstSharedFragmentDataStruct(ScriptStruct);
			if (ConstSharedFragment.IsValid())
			{
				Context.SetContextData(DataDesc.Handle, FStateTreeDataView(ConstSharedFragment.GetScriptStruct(), const_cast<uint8*>(ConstSharedFragment.GetMemory())));
			}
			else
			{
				bFoundAll = false;
			}
		}
		else
		{
			checkf(false, TEXT("Could not bind: %s"), *DataDesc.Struct->GetPathName());
		}
	}

	return bFoundAll;
}

bool CollectExternalData(const FStateTreeExecutionContext& Context, const UStateTree* StateTree, TArrayView<const FStateTreeExternalDataDesc> ExternalDataDescs, TArrayView<FStateTreeDataView> OutDataViews)
{
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(StateTreeProcessorCollectExternalData);

	const FRecallStateTreeExecutionContext& RecallContext = static_cast<const FRecallStateTreeExecutionContext&>(Context);
	const FMassEntityManager& EntityManager = RecallContext.GetEntityManager();
	const UWorld* World = RecallContext.GetWorld();

	TArray<FName> MissingDataDescs;
	const FMassEntityView EntityView(EntityManager, RecallContext.GetEntity());

	check(ExternalDataDescs.Num() == OutDataViews.Num());

	for (int32 Index = 0; Index < ExternalDataDescs.Num(); Index++)
	{
		const FStateTreeExternalDataDesc& DataDesc = ExternalDataDescs[Index];
		if (DataDesc.Struct == nullptr)
		{
			continue;
		}

		if (DataDesc.Struct->IsChildOf(FMassFragment::StaticStruct()))
		{
			const UScriptStruct* ScriptStruct = Cast<const UScriptStruct>(DataDesc.Struct);
			FStructView Fragment = EntityView.GetFragmentDataStruct(ScriptStruct);
			if (Fragment.IsValid())
			{
				OutDataViews[Index] = FStateTreeDataView(Fragment);
			}
			else
			{
				if (DataDesc.Requirement == EStateTreeExternalDataRequirement::Required)
				{
					MissingDataDescs.Add(DataDesc.Struct.GetFName());
				}
			}
		}
		else if (DataDesc.Struct->IsChildOf(FMassSharedFragment::StaticStruct()))
		{
			const UScriptStruct* ScriptStruct = Cast<const UScriptStruct>(DataDesc.Struct);
			FStructView SharedFragment = EntityView.GetSharedFragmentDataStruct(ScriptStruct);
			if (SharedFragment.IsValid())
			{
				OutDataViews[Index] = FStateTreeDataView(SharedFragment.GetScriptStruct(), SharedFragment.GetMemory());
			}
			else
			{
				FConstStructView Fragment = EntityView.GetConstSharedFragmentDataStruct(ScriptStruct);
				if (Fragment.IsValid())
				{
					OutDataViews[Index] = FStateTreeDataView(Fragment.GetScriptStruct(), const_cast<uint8*>(Fragment.GetMemory()));
				}
				else
				{
					if (DataDesc.Requirement == EStateTreeExternalDataRequirement::Required)
					{
						MissingDataDescs.Add(DataDesc.Struct.GetFName());
					}
				}
			}
		}
		else if (DataDesc.Struct->IsChildOf(FMassConstSharedFragment::StaticStruct()))
		{
			const UScriptStruct* ScriptStruct = Cast<const UScriptStruct>(DataDesc.Struct);
			FConstStructView Fragment = EntityView.GetConstSharedFragmentDataStruct(ScriptStruct);
			if (Fragment.IsValid())
			{
				OutDataViews[Index] = FStateTreeDataView(Fragment.GetScriptStruct(), const_cast<uint8*>(Fragment.GetMemory()));
			}
			else
			{
				if (DataDesc.Requirement == EStateTreeExternalDataRequirement::Required)
				{
					MissingDataDescs.Add(DataDesc.Struct.GetFName());
				}
			}
		}
		else if (DataDesc.Struct && DataDesc.Struct->IsChildOf(UWorldSubsystem::StaticClass()))
		{
			const TSubclassOf<UWorldSubsystem> SubClass = Cast<UClass>(const_cast<UStruct*>(ToRawPtr(DataDesc.Struct)));
			UWorldSubsystem* Subsystem = World->GetSubsystemBase(SubClass);
			if (Subsystem)
			{
				OutDataViews[Index] = FStateTreeDataView(Subsystem);
			}
			else
			{
				if (DataDesc.Requirement == EStateTreeExternalDataRequirement::Required)
				{
					MissingDataDescs.Add(DataDesc.Struct.GetFName());
				}
			}
		}
		else
		{
			checkNoEntry();
		}
	}

	ensureMsgf(MissingDataDescs.Num() == 0, TEXT("Missing external data"));

	return MissingDataDescs.Num() == 0;
}
}; // UE::MassBehavior

FRecallStateTreeExecutionContext::FRecallStateTreeExecutionContext(UObject& InOwner, const UStateTree& InStateTree, FStateTreeInstanceData& InInstanceData,
	FMassEntityManager& InEntityManager, URecallSignalSubsystem& InSignalSystem, FMassExecutionContext& InContext, const FMassEntityHandle& InEntity)
	: FStateTreeExecutionContext(InOwner, InStateTree, InInstanceData, FOnCollectStateTreeExternalData::CreateStatic(UE::MassBehavior::CollectExternalData))
	, EntityManager(&InEntityManager)
	, SignalSubsystem(&InSignalSystem)
	, EntitySubsystemExecutionContext(&InContext)
	, Entity(InEntity)
{
	UE::MassBehavior::SetContextData(*this);
}

void FRecallStateTreeExecutionContext::BeginDelayedTransition(const FStateTreeTransitionDelayedState& DelayedState)
{
	if (SignalSubsystem != nullptr && Entity.IsSet())
	{
		// Tick again after the games time has passed to see if the condition still holds true.
		SignalSubsystem->DelaySignalEntity(Recall::StateTree::Signals::DelayedTransitionWakeup, Entity, DelayedState.TimeLeft + UE_KINDA_SMALL_NUMBER);
	}
}
