// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "RecallSignalProcessorBase.h"

#include "MassEntityQuery.h"

#include "RecallInteractProcessors.generated.h"

/**
* Handle overlapping and tick signals for the interactor entities
*/
UCLASS()
class URecallInteractorSignalProcessor : public URecallSignalProcessorBase
{
	GENERATED_BODY()

public:
	URecallInteractorSignalProcessor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FRecallSignalNameLookup& EntitySignals) override final;
};

/**
* Handle interaction inputs
*/
UCLASS()
class URecallInteractorInputProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallInteractorInputProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

/**
* Update representation of interactions
*/
UCLASS()
class URecallInteractorRepresentationEventProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	URecallInteractorRepresentationEventProcessor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

	TSharedPtr<struct FRecallInteractorRepresentationCacheManager> CacheManager;

};
