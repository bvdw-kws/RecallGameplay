// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassProcessor.h"
#include "NativeGameplayTags.h"

#include "MassEntityQuery.h"

#include "RecallInventoryProcessors.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(StateTreeEvent_UseInventoryItem);

UCLASS()
class URecallInventoryInputProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URecallInventoryInputProcessor();

protected:
	void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& InEntityManager) override final;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override final;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override final;

private:
	FMassEntityQuery EntityQuery;

};
