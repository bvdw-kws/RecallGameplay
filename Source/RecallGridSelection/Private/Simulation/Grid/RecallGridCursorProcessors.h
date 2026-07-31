// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"

#include "MassEntityQuery.h"

#include "RecallGridCursorProcessors.generated.h"

UCLASS()
class URecallGridCursorOwnerInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallGridCursorOwnerInitializer();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridCursorOwnerDeinitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallGridCursorOwnerDeinitializer();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridSelectionInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URecallGridSelectionInitializer();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridCursorOwnerInputProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallGridCursorOwnerInputProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridSelectionPositionProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallGridSelectionPositionProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridSelectionRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallGridSelectionRepresentationProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridCursorPositionProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallGridCursorPositionProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallGridCursorRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallGridCursorRepresentationProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};
