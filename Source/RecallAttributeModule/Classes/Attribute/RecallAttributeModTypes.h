// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "RecallAttributeModTypes.generated.h"

struct FRecallAttributeSet;

/**
 * Attribute modifier that can be applied to an entity's attribute set.
 */
UCLASS(Abstract, Blueprintable, DisplayName="Attribute Mod")
class RECALLATTRIBUTEMODULE_API URecallAttributeMod : public UObject
{
	GENERATED_BODY()

public:
	void ApplyModBaseValue(const FRecallAttributeSet& AttributeSet, const FGameplayTag& Attribute, float& BaseValue) const;
	void ApplyModValue(const FRecallAttributeSet& AttributeSet, const FGameplayTag& Attribute, float& BaseValue) const;
	void ApplyModInstant(FRecallAttributeSet& AttributeSet) const;

	int32 GetModPriority() const { return ModPriority; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Attribute Mod", DisplayName="Priority")
	int32 ModPriority;

	UPROPERTY(Instanced, EditDefaultsOnly, Category="Attribute Mod", DisplayName="Operations")
	TArray<class URecallAttributeModOperationFloat*> ModOperations;

private:
	void ApplyModValue_Internal(const FRecallAttributeSet& AttributeSet, const FGameplayTag& Attribute,
		float& Value, bool bIsBaseValue) const;
};
