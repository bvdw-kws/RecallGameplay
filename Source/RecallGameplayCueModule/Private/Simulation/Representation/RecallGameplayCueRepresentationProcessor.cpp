// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#include "RecallGameplayCueRepresentationProcessor.h"

#include "MassExecutionContext.h"
#include "Simulation/Representation/RecallActorRepresentationProcessorGroups.h"
#include "System/RecallGameplayCueSubsystem.h"

URecallGameplayCueRepresentationProcessor::URecallGameplayCueRepresentationProcessor()
	: Super()
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::Render;
	ExecutionOrder.ExecuteAfter.Add(Recall::Gameplay::ProcessorGroupNames::ActorRepresentation);
	bRequiresGameThreadExecution = true;
}

bool URecallGameplayCueRepresentationProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	// This processor has no entity query: it reconciles subsystem state even in an empty Mass world.
	return false;
}

void URecallGameplayCueRepresentationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ProcessorRequirements.AddSubsystemRequirement<URecallGameplayCueSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallGameplayCueRepresentationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	URecallGameplayCueSubsystem& CueSystem = Context.GetMutableSubsystemChecked<URecallGameplayCueSubsystem>();
	CueSystem.ReconcileRepresentations();
}
