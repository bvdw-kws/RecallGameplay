// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Mass/ExternalSubsystemTraits.h"
#include "RecallNavigationTypes.h"

#include "RecallNavigationSubsystem.generated.h"

UCLASS()
class RECALLNAVIGATION_API URecallNavigationSubsystem :
	public UWorldSubsystem,
	public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	bool RequestPathSync(const FVector& Start, const FVector& End, TArray<FRecallNavigationPathPoint>& OutPathPoints, float Radius = -1.f, float Height = -1.f);

	void RequestPathAsync(FRecallNavigationHandle& Handle, const FVector& Start, const FVector& End, float Radius = -1.f, float Height = -1.f);
	bool IsRequestFinished(const FRecallNavigationHandle& Handle) const;
	void ReleaseAsyncPath(FRecallNavigationHandle& Handle);
	bool GetAsyncPathResult(const FRecallNavigationHandle& Handle, TArray<FRecallNavigationPathPoint>& OutPathPoints) const;

	void TickPathQueue();

public:
	// UWorldSubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override final;
	virtual void Deinitialize() override final;
	// UWorldSubsystem implementation End

	// IRecallSimulationReactSystemInterface implementation Begin
	virtual void Reset() override final;
	virtual void Save(const FRecallSnapshotContext& Context, FInstancedStruct& OutSnapshot) override final;
	virtual void Restore(const FRecallSnapshotContext& Context, const FInstancedStruct& InSnapshot) override final;
	// IRecallSimulationReactSystemInterface implementation End

private:
	UPROPERTY(Transient)
	TObjectPtr<URecallNavigationGamethreadQueue> NavigationGamethread;
	UPROPERTY(Transient)
	uint32 SerialNumberGenerator = 0;
	UPROPERTY(Transient)
	TMap<FRecallNavigationHandle, FRecallNavigationData> NavigationDataMap;

	mutable FCriticalSection DataGuard;

	void OnTickStart(float DeltaTime);

	int32 GetConcurrentPath(uint32 Frame) const;
	int32 GetPathWaitDuration() const;

	void OnWorldPreBeginPlay();
};

template<>
struct TMassExternalSubsystemTraits<URecallNavigationSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
