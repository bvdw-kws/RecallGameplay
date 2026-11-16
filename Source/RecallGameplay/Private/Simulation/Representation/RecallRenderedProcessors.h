// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"

#include "MassEntityQuery.h"

#include "RecallRenderedProcessors.generated.h"

/**
 * Adds / removes FRecallRenderedTag on entities tagged FRecallCullableActorRepresentationTag, based on the player cameras.
 * Runs in the simulation so the result is deterministic and safe with rollback.
 *
 * A cullable entity is considered rendered when:
 *  - any view camera (camera of a controller with FRecallViewControllerTag) has its render filter Off, or
 *  - it is inside the render box of the camera of any view controller, or
 *  - it has attached actors.
 */
UCLASS()
class URecallRenderedTagProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallRenderedTagProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	/** Controllers that are a view, they give us the cameras to render around. */
	FMassEntityQuery ViewControllerQuery;

	/** Cameras, filtered to the ones of the view controllers. */
	FMassEntityQuery CameraQuery;

	/** Entities that are currently not rendered. */
	FMassEntityQuery NotRenderedQuery;

	/** Entities that are currently rendered. */
	FMassEntityQuery RenderedQuery;
};

/**
 * Releases the actor representation when an entity stops being rendered.
 * The spawn counterpart is URecallActorRepresentationInitializer, which observes the tag being added.
 */
UCLASS()
class URecallActorRepresentationCullingDeinitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallActorRepresentationCullingDeinitializer();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};
