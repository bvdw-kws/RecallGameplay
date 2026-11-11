// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Mass/EntityElementTypes.h"
#include "MassEntityTypes.h"
#include "Gameplay/RecallGameplayEffectContainer.h"
#include "Templates/SubclassOf.h"

#include "RecallGameplayEffectFragments.generated.h"

class URecallGameplayEffect;

/**
 * Allows an entity to hold gameplay effects.
 */
USTRUCT()
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FRecallGameplayEffectContainer Container;
};

template <>
struct TMassFragmentTraits<FRecallGameplayEffectFragment> final
{ enum { AuthorAcceptsItsNotTriviallyCopyable = true }; };

/** Gameplay effects configured as defaults for entities using the gameplay effect trait. */
USTRUCT()
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectConstSharedFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<TSubclassOf<URecallGameplayEffect>> DefaultEffects;
};

USTRUCT()
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectAreaInstance
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FRecallGameplayEffectHandle EffectHandle;
};

USTRUCT()
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectAreaFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TSubclassOf<URecallGameplayEffect> EffectClass;
	
	UPROPERTY(VisibleAnywhere)
	TMap<FMassEntityHandle, FRecallGameplayEffectAreaInstance> InstanceMap;
};

template <>
struct TMassFragmentTraits<FRecallGameplayEffectAreaFragment> final
{ enum { AuthorAcceptsItsNotTriviallyCopyable = true }; };
