// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallRenderedProcessors.h"

#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "Simulation/Controller/RecallControllerFragments.h"
#include "Simulation/Player/Camera/RecallPlayerCameraFragments.h"
#include "Simulation/Player/Camera/RecallPlayerCameraProcessorGroupTypes.h"
#include "Simulation/Representation/RecallActorRepresentationFragments.h"
#include "Simulation/Transform/RecallTransformFragments.h"
#include "System/Actor/RecallActorSubsystem.h"
#include "System/Representation/RecallActorInterpolationSubsystem.h"

namespace Recall::Gameplay::Private
{
	/** Render area of a player camera. */
	struct FCameraRenderBox
	{
		FBox Box = FBox(ForceInit);

		/** Bigger box used by entities already rendered, to avoid flickering at the border. */
		FBox HysteresisBox = FBox(ForceInit);
	};
}

//----------------------------------------------------------------------//
// URecallRenderedTagProcessor
//----------------------------------------------------------------------//
URecallRenderedTagProcessor::URecallRenderedTagProcessor()
	: ViewControllerQuery(*this)
	, CameraQuery(*this)
	, NotRenderedQuery(*this)
	, RenderedQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ProcessingPhase = EMassProcessingPhase::FrameEnd;
	// The camera transform is final once the follow processor is done.
	ExecutionOrder.ExecuteAfter.Add(Recall::Camera::ProcessorGroupNames::Follow);
}

void URecallRenderedTagProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallRenderedTagProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	ViewControllerQuery.AddRequirement<FRecallControllerFragment>(EMassFragmentAccess::ReadOnly);
	ViewControllerQuery.AddTagRequirement<FRecallViewControllerTag>(EMassFragmentPresence::All);

	CameraQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	CameraQuery.AddRequirement<FRecallPlayerCameraFragment>(EMassFragmentAccess::ReadOnly);
	CameraQuery.AddConstSharedRequirement<FRecallPlayerCameraSharedFragment>();

	NotRenderedQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	NotRenderedQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	NotRenderedQuery.AddTagRequirement<FRecallCullableActorRepresentationTag>(EMassFragmentPresence::All);
	NotRenderedQuery.AddTagRequirement<FRecallRenderedTag>(EMassFragmentPresence::None);

	RenderedQuery.AddRequirement<FRecallTransformFragment>(EMassFragmentAccess::ReadOnly);
	RenderedQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	RenderedQuery.AddTagRequirement<FRecallCullableActorRepresentationTag>(EMassFragmentPresence::All);
	RenderedQuery.AddTagRequirement<FRecallRenderedTag>(EMassFragmentPresence::All);
}

void URecallRenderedTagProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_Rendered_Tag);

	using namespace Recall::Gameplay::Private;

	// Cameras of the controllers that are a view.
	TSet<FMassEntityHandle> ViewCameraEntities;

	ViewControllerQuery.ForEachEntityChunk(Context, [&ViewCameraEntities](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRecallControllerFragment> ControllerList = Context.GetFragmentView<FRecallControllerFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			const FRecallControllerFragment& ControllerFragment = ControllerList[EntityIndex];
			if (ControllerFragment.CameraEntity.IsSet())
			{
				ViewCameraEntities.Add(ControllerFragment.CameraEntity);
			}
		}
	});

	// Collect the area rendered by each view camera.
	TArray<FCameraRenderBox> RenderBoxes;
	bool bRenderEverything = false;

	CameraQuery.ForEachEntityChunk(Context, [&ViewCameraEntities, &RenderBoxes, &bRenderEverything](FMassExecutionContext& Context)
	{
		const FRecallPlayerCameraSharedFragment& CameraSharedFragment = Context.GetConstSharedFragment<FRecallPlayerCameraSharedFragment>();
		const FRecallPlayerCameraSettings& Settings = CameraSharedFragment.Settings;

		const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			if (!ViewCameraEntities.Contains(Context.GetEntity(EntityIndex)))
			{
				continue;
			}

			if (Settings.RenderFilter == ERecallCameraRenderFilter::Off)
			{
				bRenderEverything = true;
				return;
			}

			const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];

			const FVector Center = TransformFragment.Position;
			const FVector Extent = Settings.RenderBoxExtent;
			const FVector HysteresisExtent = Extent + FVector(Settings.RenderBoxHysteresis);

			FCameraRenderBox& RenderBox = RenderBoxes.AddDefaulted_GetRef();
			RenderBox.Box = FBox(Center - Extent, Center + Extent);
			RenderBox.HysteresisBox = FBox(Center - HysteresisExtent, Center + HysteresisExtent);
		}
	});

	// Without any view camera nothing is rendered, unless one of them is unfiltered (everything is rendered).

	auto ProcessEntities = [&RenderBoxes, &bRenderEverything](FMassEntityQuery& Query, FMassExecutionContext& Context, const bool bCurrentlyRendered)
	{
		Query.ForEachEntityChunk(Context, [&RenderBoxes, bRenderEverything, bCurrentlyRendered](FMassExecutionContext& Context)
		{
			const TConstArrayView<FRecallTransformFragment> TransformList = Context.GetFragmentView<FRecallTransformFragment>();
			const TConstArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetFragmentView<FRecallActorRepresentationFragment>();

			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
			{
				const FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];

				bool bShouldBeRendered = true;

				// Attached actors would be left behind if we released the parent actor.
				const bool bHasAttachments = !ActorFragment.AttachmentActorHandles.IsEmpty();

				if (!bHasAttachments && !bRenderEverything)
				{
					const FRecallTransformFragment& TransformFragment = TransformList[EntityIndex];
					const FVector& Position = TransformFragment.Position;

					bShouldBeRendered = false;
					for (const FCameraRenderBox& RenderBox : RenderBoxes)
					{
						if ((bCurrentlyRendered ? RenderBox.HysteresisBox : RenderBox.Box).IsInside(Position))
						{
							bShouldBeRendered = true;
							break;
						}
					}
				}

				if (bShouldBeRendered == bCurrentlyRendered)
				{
					continue;
				}

				if (bShouldBeRendered)
				{
					Context.Defer().AddTag<FRecallRenderedTag>(Context.GetEntity(EntityIndex));
				}
				else
				{
					Context.Defer().RemoveTag<FRecallRenderedTag>(Context.GetEntity(EntityIndex));
				}
			}
		});
	};

	ProcessEntities(NotRenderedQuery, Context, false);
	ProcessEntities(RenderedQuery, Context, true);
}

//----------------------------------------------------------------------//
// URecallActorRepresentationCullingDeinitializer
//----------------------------------------------------------------------//
URecallActorRepresentationCullingDeinitializer::URecallActorRepresentationCullingDeinitializer()
	: EntityQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
	ObservedTypes.Add(FRecallRenderedTag::StaticStruct());
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void URecallActorRepresentationCullingDeinitializer::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager)
{
	Super::InitializeInternal(Owner, InEntityManager);
}

void URecallActorRepresentationCullingDeinitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FRecallActorRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<URecallActorSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FRecallCullableActorRepresentationTag>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<URecallActorInterpolationSubsystem>(EMassFragmentAccess::ReadWrite);
}

void URecallActorRepresentationCullingDeinitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	QUICK_SCOPE_CYCLE_COUNTER(Recall_ActorRepresentation_CullingDeinitializer);

	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		URecallActorSubsystem& ActorSystem = Context.GetMutableSubsystemChecked<URecallActorSubsystem>();
		URecallActorInterpolationSubsystem& ActorInterpolationSystem = Context.GetMutableSubsystemChecked<URecallActorInterpolationSubsystem>();

		const TArrayView<FRecallActorRepresentationFragment> ActorList = Context.GetMutableFragmentView<FRecallActorRepresentationFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); EntityIndex++)
		{
			FRecallActorRepresentationFragment& ActorFragment = ActorList[EntityIndex];

			ActorInterpolationSystem.ClearInterpolationCache(ActorFragment.ActorHandle);
			ActorSystem.ReleaseActor(ActorFragment.ActorHandle);
		}
	});
}
