// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/GameplayTag/RecallGameplayTagUtils.h"

#include "Data/Device/RecallDeviceAsset.h"
#include "Data/Faction/RecallFactionTypes.h"
#include "Data/GameplayTag/RecallGameplayTagConditionTypes.h"
#include "GameplayTag/RecallGameplayTagTypes.h"

namespace Recall::GameplayTag::Utils
{

bool EvaluateCondition(const FRecallGameplayTagCondition& Condition, const FRecallGameplayTagCountMap& GameplayTagCountMap)
{
	// Check required tags based on matching mode
	if (Condition.MatchingMode == ERecallGameplayTagMatchingMode::Hierarchical)
	{
		// For hierarchical matching, check if entity has any child tags of the required parent tags
		const FGameplayTagContainer EntityTags = GameplayTagCountMap.GetTags();
		for (const FGameplayTag& RequiredTag : Condition.RequiredTags)
		{
			bool bHasMatchingTag = false;
			for (const FGameplayTag& EntityTag : EntityTags)
			{
				if (EntityTag.MatchesTag(RequiredTag))
				{
					bHasMatchingTag = true;
					break;
				}
			}
			if (!bHasMatchingTag)
			{
				return false;
			}
		}
	}
	else
	{
		// Exact matching (default behavior)
		if (!GameplayTagCountMap.HasAllMatchingGameplayTags(Condition.RequiredTags))
		{
			return false;
		}
	}

	// Check forbidden tags based on matching mode
	if (Condition.MatchingMode == ERecallGameplayTagMatchingMode::Hierarchical)
	{
		// For hierarchical matching, check if entity has any child tags of the forbidden parent tags
		const FGameplayTagContainer EntityTags = GameplayTagCountMap.GetTags();
		for (const FGameplayTag& ForbiddenTag : Condition.ForbiddenTags)
		{
			for (const FGameplayTag& EntityTag : EntityTags)
			{
				if (EntityTag.MatchesTag(ForbiddenTag))
				{
					return false;
				}
			}
		}
	}
	else
	{
		// Exact matching (default behavior)
		if (GameplayTagCountMap.HasAnyMatchingGameplayTags(Condition.ForbiddenTags))
		{
			return false;
		}
	}

	return true;
}

FGameplayTagContainer GetFactionTags(const FRecallGameplayTagCountMap& GameplayTagCountMap)
{
	return GetFactionTags(GameplayTagCountMap.GetTags());
}

FGameplayTagContainer GetFactionTags(const FGameplayTagContainer& GameplayTags)
{
	return GameplayTags.Filter(Faction_Parent.GetTag().GetSingleTagContainer());;
}

FGameplayTagContainer GetDeviceTags(const FRecallGameplayTagCountMap& GameplayTagCountMap)
{
	return GetDeviceTags(GameplayTagCountMap.GetTags());
}

FGameplayTagContainer GetDeviceTags(const FGameplayTagContainer& GameplayTags)
{
	return GameplayTags.Filter(Device_Parent.GetTag().GetSingleTagContainer());
}

bool IsSameFaction(
	const FRecallGameplayTagCountMap& lGameplayTagCountMap, const FRecallGameplayTagCountMap& rGameplayTagCountMap)
{
	const FGameplayTagContainer lFactionTags = GetFactionTags(lGameplayTagCountMap.GetTags());
	const FGameplayTagContainer rFactionTags = GetFactionTags(rGameplayTagCountMap.GetTags());

	return lFactionTags.HasAll(rFactionTags);
}
	
} // namespace Recall::GameplayTag::Utils
