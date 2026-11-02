// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

#pragma once

#include "CoreMinimal.h"
#include "System/RecallGameplayCueTypes.h"

#include "RecallGameplayCueSnapshot.generated.h"

USTRUCT()
struct FRecallGameplayCueSnapshot
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FRecallGameplayCueInstance> ActiveCueInstances;

	UPROPERTY()
	uint64 CueInstanceIdGenerator = 0;
};
