// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FRecallGameplayTagCondition;
struct FRecallGameplayTagCountMap;

namespace Recall::GameplayTag::Utils
{

RECALLGAMEPLAY_API extern bool EvaluateCondition(
	const FRecallGameplayTagCondition& Condition, const FRecallGameplayTagCountMap& GameplayTagCountMap);
	
RECALLGAMEPLAY_API extern FGameplayTagContainer GetFactionTags(const FRecallGameplayTagCountMap& GameplayTagCountMap);
RECALLGAMEPLAY_API extern FGameplayTagContainer GetFactionTags(const FGameplayTagContainer& GameplayTags);
RECALLGAMEPLAY_API extern FGameplayTagContainer GetDeviceTags(const FRecallGameplayTagCountMap& GameplayTagCountMap);
RECALLGAMEPLAY_API extern FGameplayTagContainer GetDeviceTags(const FGameplayTagContainer& GameplayTags);
RECALLGAMEPLAY_API extern bool IsSameFaction(
	const FRecallGameplayTagCountMap& lGameplayTagCountMap, const FRecallGameplayTagCountMap& rGameplayTagCountMap);
	
} // namespace Recall::GameplayTag::Utils
