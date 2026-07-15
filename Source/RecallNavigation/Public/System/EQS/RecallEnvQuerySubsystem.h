// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "System/Interface/RecallSimulationReactSystemInterface.h"
#include "Mass/ExternalSubsystemTraits.h"
#include "RecallEnvQueryTypes.h"

#include "RecallEnvQuerySubsystem.generated.h"

UCLASS()
class RECALLNAVIGATION_API URecallEnvQuerySubsystem : 
	public UWorldSubsystem,
	public IRecallSimulationReactSystemInterface
{
	GENERATED_BODY()

public:
	FRecallEnvQueryHandle RequestEnvQuery(const FMassEntityHandle& Entity, const FRecallEnvQueryRequest& Request);
	bool IsEnvQueryFinished(const FRecallEnvQueryHandle& Handle) const;
	bool GetEnvQueryBestResult(const FRandomStream& RandomStream, FRecallEnvQueryHandle& Handle, FVector& OutLocation);
	void ReleaseEnvQuery(FRecallEnvQueryHandle& Handle);

	void TickEnvQueryQueue();
	
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	const TMap<FRecallEnvQueryHandle, FRecallEnvQueryDebugCache>& GetEnvQueryDebugCache() const { return EnvQueryDebugCache; }
#endif // UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	
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
	TWeakObjectPtr<class URecallSignalSubsystem> SignalSystem;
	UPROPERTY(Transient)
	TWeakObjectPtr<class URecallRandomNumberSubsystem> RandomNumberSystem;
	UPROPERTY(Transient)
	TObjectPtr<URecallEnvQueryGamethreadQueue> EnvQueryGamethread;
	UPROPERTY(Transient)
	int32 LastTickEnvQueryIndex = 0;

	UPROPERTY(Transient)
	uint32 SerialNumberGenerator = 0;
	UPROPERTY(Transient)
	TMap<FRecallEnvQueryHandle, FRecallEnvQueryData> EnvQueryDataMap;

	UPROPERTY(Transient)
	TMap<FRecallEnvQueryHandle, FRecallEnvQueryDebugCache> EnvQueryDebugCache;

	mutable FCriticalSection EnvQueryGuard;

	void OnTickStart(float DeltaTime);

	int32 GetConcurrentEnvQuery(uint32 Frame) const;
	int32 GetEnvQueryDuration() const;
};

template<>
struct TMassExternalSubsystemTraits<URecallEnvQuerySubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
