// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RecallGameplayEffectTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"

#include "RecallGameplayEffect.generated.h"

class URecallAttributeMod;

/**
 * Gameplay cues associated with a Recall gameplay effect.
 * Persistent effects add these cues while active; instant effects execute them once.
 */
USTRUCT(BlueprintType)
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectCue
{
	GENERATED_BODY()

	/** Tags sent to the Gameplay Cue handler for this cue entry. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GameplayCue, meta=(Categories="GameplayCue"))
	FGameplayTagContainer GameplayCueTags;

	/** Fixed normalized magnitude supplied to the Gameplay Cue handler. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GameplayCue, meta=(ClampMin="0.0", ClampMax="1.0"))
	float NormalizedMagnitude = 1.0f;

	/** Fixed raw magnitude supplied to the Gameplay Cue handler. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GameplayCue)
	float RawMagnitude = 0.0f;
};

/**
 * Gameplay effect that can be applied to an entity.
 */
UCLASS(Abstract, Blueprintable, DisplayName="Gameplay Effect")
class RECALLGAMEPLAYEFFECTMODULE_API URecallGameplayEffect : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Name of the gameplay effect to display in the UI.
	 */
	UPROPERTY(EditAnywhere, Category=UI)
	FText Name;

	/**
	 * Description of the gameplay effect to display in the UI.
	 */
	UPROPERTY(EditAnywhere, Category=UI)
	FText Description;

	/**
	 * Icon of the gameplay effect to display in the UI.
	 */
	UPROPERTY(EditAnywhere, Category=UI)
	TSoftObjectPtr<UTexture2D> Icon;
	
	/**
	 * Tag used to identify this effect.
	 */
	UPROPERTY(EditAnywhere, Category=Effect, meta=(GameplayTagFilter="Effect"))
	FGameplayTag EffectTag;
	
	/**
	 * Gameplay tags to add while this effect is active.
	 */
	UPROPERTY(EditAnywhere, Category=Effect)
	FGameplayTagContainer GameplayTagsToAdd;

	/** Cues to activate while this effect is active, or execute once for instant effects. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GameplayCues)
	TArray<FRecallGameplayEffectCue> GameplayCues;

	/**
	 * Attribute modifiers to apply while this effect is active.
	 */
	UPROPERTY(EditAnywhere, Category=Effect)
	TArray<TSubclassOf<URecallAttributeMod>> AttributeMods;

	/**
	 * Whether this effect can be stacked per class.
	 * When false, effects of the same class will each be considered as unique.
	 */
	UPROPERTY(EditAnywhere, Category=Stack)
	bool bUseStack = false;

	/**
	 * Up to how many times this gameplay effect can be stacked.
	 * Past this limit, older instances will be removed so new ones can be added.
	 */
	UPROPERTY(EditAnywhere, Category=Stack, meta=(EditCondition="bUseStack", ClampMin=1, ClampMax=999))
	int32 StackLimit = 1;

	/**
	 * Define how the stack of gameplay effects is managed.
	 */
	UPROPERTY(EditAnywhere, Category=Stack, meta=(EditCondition="bUseStack"))
	ERecallGameplayEffectStackRule StackRule = ERecallGameplayEffectStackRule::Shared;

	/** Policy for the duration of this effect */
	UPROPERTY(EditAnywhere, Category=Duration)
	ERecallGameplayEffectDurationType DurationPolicy = ERecallGameplayEffectDurationType::HasDuration;
	
	/**
	 * Duration of this gameplay effect.
	 * 0 means that it is infinite.
	 */
	UPROPERTY(EditAnywhere, Category=Duration, meta=(ClampMin=0.0f, Units=Seconds, EditCondition="DurationPolicy == ERecallGameplayEffectDurationType::HasDuration"))
	float Duration = 0.0f;
	
	/**
	 * Rule that defines how this gameplay effect is ticked.
	 */
	UPROPERTY(EditAnywhere, Category=Duration, meta=(EditCondition="DurationPolicy == ERecallGameplayEffectDurationType::HasDuration"))
	ERecallGameplayEffectTickRule TickRule = ERecallGameplayEffectTickRule::Default;
};
