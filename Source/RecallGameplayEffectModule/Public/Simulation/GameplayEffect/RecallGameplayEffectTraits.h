// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "MassEntityTraitBase.h"
#include "Templates/SubclassOf.h"

#include "RecallGameplayEffectTraits.generated.h"

class URecallGameplayEffect;

UCLASS(meta=(DisplayName="RE Gameplay Effect"))
class RECALLGAMEPLAYEFFECTMODULE_API URecallGameplayEffectTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	/** Gameplay effects to apply when an entity with this trait is initialized. */
	UPROPERTY(EditAnywhere, Category=GameplayEffect)
	TArray<TSubclassOf<URecallGameplayEffect>> DefaultEffects;
};

UCLASS(meta=(DisplayName="RE Gameplay Effect Area"))
class RECALLGAMEPLAYEFFECTMODULE_API URecallGameplayEffectAreaTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

protected:
	/**
	 * Effect applied by this area.
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<URecallGameplayEffect> EffectClass;
};
