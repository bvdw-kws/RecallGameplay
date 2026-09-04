// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"

#include "MassEntityQuery.h"

#include "RecallNavigationObstacleProcessors.generated.h"

UCLASS()
class URecallNavigationObstacleInitializeProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallNavigationObstacleInitializeProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallNavigationObstacleResetProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallNavigationObstacleResetProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;
};
