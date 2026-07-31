// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"

#include "MassEntityQuery.h"

#include "RecallGameRuleCommonCommandProcessors.generated.h"

/**
 * Processor for destroy entities commands
 */
UCLASS()
class URecallGameRuleDestroyEntitiesProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    URecallGameRuleDestroyEntitiesProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    
private:
    FMassEntityQuery EntityQuery;
};

/**
 * Processor for apply tag commands
 */
UCLASS()
class URecallGameRuleApplyTagProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    URecallGameRuleApplyTagProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    
private:
    FMassEntityQuery EntityQuery;
};

/**
 * Processor for swap positions commands
 */
UCLASS()
class URecallGameRuleSwapPositionsProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    URecallGameRuleSwapPositionsProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
    
private:
    FMassEntityQuery EntityQuery;
};

/**
 * Processor for move to position commands (e.g., Mouse Monopoly)
 */
UCLASS()
class URecallGameRuleMoveToPositionProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    URecallGameRuleMoveToPositionProcessor();

protected:
    virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
        
private:
    FMassEntityQuery EntityQuery;
};
