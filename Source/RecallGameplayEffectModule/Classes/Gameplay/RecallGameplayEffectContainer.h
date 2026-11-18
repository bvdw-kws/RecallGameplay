// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "RecallGameplayEffectContext.h"
#include "RecallGameplayEffectHandle.h"
#include "RecallGameplayEffectInstance.h"
#include "Templates/SubclassOf.h"

#include "RecallGameplayEffectContainer.generated.h"

struct FGameplayTag;
struct FMassEntityView;
struct FRecallGameplayEffectContext;

USTRUCT()
struct RECALLGAMEPLAYEFFECTMODULE_API FRecallGameplayEffectContainer
{
	GENERATED_BODY()

public:
	virtual ~FRecallGameplayEffectContainer() = default;
	
public:
	void Initialize(const FMassEntityHandle& InOwnerEntity);

public:
	FRecallGameplayEffectHandle AddEffect(const FRecallGameplayEffectContext& Context, const TSubclassOf<URecallGameplayEffect>& Effect);
	void AddEffects(const FRecallGameplayEffectContext& Context, const TArray<TSubclassOf<URecallGameplayEffect>>& Effects);

	bool RemoveEffect(const FRecallGameplayEffectContext& Context, const FRecallGameplayEffectHandle& Handle);
	bool RemoveEffectsByClass(const FRecallGameplayEffectContext& Context, const TSubclassOf<URecallGameplayEffect>& Effect);
	void RemoveEffectsByClasses(const FRecallGameplayEffectContext& Context, const TArray<TSubclassOf<URecallGameplayEffect>>& Effects);
	bool RemoveEffectsByTag(const FRecallGameplayEffectContext& Context, const FGameplayTag& EffectTag);

	TArray<TSubclassOf<URecallGameplayEffect>> GetEffectClasses() const;
	TArray<FRecallGameplayEffectHandle> GetAllEffectHandles(const TSubclassOf<URecallGameplayEffect>& Effect,
		const FMassEntityHandle& EffectOwnerEntity = FMassEntityHandle()) const;

	void TickEffects(const FRecallGameplayEffectContext& Context, float DeltaTime,
		ERecallGameplayEffectTickSource Source = ERecallGameplayEffectTickSource::Custom);

protected:
	/**
	 * Entity owning this modifier to whom the effect is being applied.
	 */
	UPROPERTY(VisibleAnywhere)
	FMassEntityHandle OwnerEntity;

	/**
	 * Map of effects being applied to the owning entity.
	 */
	UPROPERTY(VisibleAnywhere)
	TMap<FRecallGameplayEffectHandle, FRecallGameplayEffectInstance> EffectInstanceMap;

	/**
	 * Unique Id generator used to generate effect handles.
	 */
	UPROPERTY(VisibleAnywhere)
	uint32 SerialNumberGenerator = 0;

protected:
	virtual void AddEffect_Internal(const FRecallGameplayEffectContext& Context,
		const URecallGameplayEffect& EffectCDO, FRecallGameplayEffectInstance* Instance) const;
	void AddGameplayCue_Internal(const FRecallGameplayEffectContext& Context,
		const URecallGameplayEffect& EffectCDO, const FMassEntityView& OwnerView,
		FRecallGameplayEffectInstance* Instance, bool bInstant) const;
	virtual void RemoveEffect_Internal(const FRecallGameplayEffectContext& Context, const FRecallGameplayEffectInstance& Instance) const;

private:
	/**
	 * Internally handle the stack for this effect BEFORE it is newly added.
	 */
	void HandleEffectStack(const FRecallGameplayEffectContext& Context, const TSubclassOf<URecallGameplayEffect>& Effect);

	/**
	 * Compute the duration of this effect: base duration with the duration operations applied using the owner's attributes.
	 */
	float ComputeDuration(const FRecallGameplayEffectContext& Context, const URecallGameplayEffect& EffectCDO) const;

	FRecallGameplayEffectHandle GetOldestEffectHandle(const TSubclassOf<URecallGameplayEffect>& Effect,
		const FMassEntityHandle& EffectOwnerEntity, int32& OutEffectOwnedCount) const;

	static bool TickEffect_Internal(float DeltaTime, ERecallGameplayEffectTickSource Source,
	                                FRecallGameplayEffectInstance& Instance);
};
