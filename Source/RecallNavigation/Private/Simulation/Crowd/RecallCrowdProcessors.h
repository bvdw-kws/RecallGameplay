// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassObserverProcessor.h"

#include "MassEntityQuery.h"

#include "RecallCrowdProcessors.generated.h"

UCLASS()
class URecallCrowdAgentConstructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	URecallCrowdAgentConstructor();

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

UCLASS()
class URecallCrowdAgentDestructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	URecallCrowdAgentDestructor();

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

UCLASS()
class URecallCrowdAgentNavLinkTraversalConstructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	URecallCrowdAgentNavLinkTraversalConstructor();

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

UCLASS()
class URecallCrowdAgentNavLinkTraversalDestructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	URecallCrowdAgentNavLinkTraversalDestructor();

	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};

UCLASS()
class URecallCrowdAgentSetBlackboardProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallCrowdAgentSetBlackboardProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallCrowdAgentGetBlackboardProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallCrowdAgentGetBlackboardProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;
};

UCLASS()
class URecallCrowdUpdateProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallCrowdUpdateProcessor();

public:
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;
};
