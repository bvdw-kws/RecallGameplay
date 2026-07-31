// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"
#include "RecallSignalProcessorBase.h"

#include "MassEntityQuery.h"

#include "RecallEquipmentProcessors.generated.h"

UCLASS()
class URecallEquipmentConstructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	URecallEquipmentConstructor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

UCLASS()
class URecallEquipmentDestructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	URecallEquipmentDestructor();

	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

/**
 * Processor to manage entity equipment based on various signals.
 */
UCLASS()
class URecallEquipmentSignalProcessor : public URecallSignalProcessorBase
{
	GENERATED_BODY()

public:
	URecallEquipmentSignalProcessor(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FRecallSignalNameLookup& EntitySignals) override final;
};

UCLASS()
class URecallEquipmentRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallEquipmentRepresentationProcessor();

protected:
	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};
