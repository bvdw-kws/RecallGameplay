// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Mod/RecallModOperationFloat.h"

#include "RecallAttributeModOperationTypes.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced, NotBlueprintable, DisplayName="Attribute Mod Operation Float")
class RECALLATTRIBUTEMODULE_API URecallAttributeModOperationFloat : public URecallModOperationFloatBase
{
	GENERATED_BODY()

public:
	FORCEINLINE bool IsBaseValueMod() const { return bApplyToBaseValue; }
	FORCEINLINE const FGameplayTag& GetTargetAttribute() const { return Attribute; }

protected:
	virtual const FGameplayTag& GetContextAttribute() const override { return Attribute; }

protected:
	UPROPERTY(EditAnywhere, Category="Attribute", meta=(GameplayTagFilter="Attribute"))
	FGameplayTag Attribute = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, Category="Attribute")
	bool bApplyToBaseValue = false;
};
