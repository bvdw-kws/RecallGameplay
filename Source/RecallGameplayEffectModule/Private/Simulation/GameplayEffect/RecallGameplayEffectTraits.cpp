// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Simulation/GameplayEffect/RecallGameplayEffectTraits.h"

#include "MassEntityTemplateRegistry.h"
#include "Simulation/GameplayEffect/RecallGameplayEffectFragments.h"
#include "Simulation/Physics/RecallPhysicsSensorFragment.h"
#include "Simulation/Transform/RecallTransformFragments.h"

//----------------------------------------------------------------------//
// URecallGameplayEffectTrait
//----------------------------------------------------------------------//
void URecallGameplayEffectTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.AddFragment<FRecallGameplayEffectFragment>();

	FRecallGameplayEffectConstSharedFragment SharedFragment;
	SharedFragment.DefaultEffects = DefaultEffects;
	BuildContext.AddConstSharedFragment(EntityManager.GetOrCreateConstSharedFragment(SharedFragment));
}

//----------------------------------------------------------------------//
// URecallGameplayEffectAreaTrait
//----------------------------------------------------------------------//
void URecallGameplayEffectAreaTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.RequireFragment<FRecallTransformFragment>();
	BuildContext.RequireFragment<FRecallPhysicsSensorFragment>();
	
	auto& GameplayEffectAreaFragment = BuildContext.AddFragment_GetRef<FRecallGameplayEffectAreaFragment>();
	GameplayEffectAreaFragment.EffectClass = EffectClass;
}
