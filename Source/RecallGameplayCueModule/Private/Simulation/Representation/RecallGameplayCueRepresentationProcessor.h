// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "MassProcessor.h"

#include "RecallGameplayCueRepresentationProcessor.generated.h"

UCLASS()
class URecallGameplayCueRepresentationProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallGameplayCueRepresentationProcessor();

public:
	virtual bool ShouldAllowQueryBasedPruning(const bool bRuntimeMode = true) const override final;

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;
};
