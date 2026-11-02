// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/RecallStateTreeTaskBase.h"
#include "UObject/SoftObjectPtr.h"

#include "RecallAudioTasks.generated.h"

class USoundBase;

USTRUCT()
struct RECALLGAMEPLAY_API FRecallPlaySoundTaskInstanceData
{
	GENERATED_BODY()
};

/** Queues a sound to play after the current simulation frame is confirmed. */
USTRUCT(meta=(DisplayName="Play Sound"))
struct RECALLGAMEPLAY_API FRecallPlaySoundTask : public FRecallStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallPlaySoundTaskInstanceData;

public:
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	UPROPERTY(EditAnywhere, Category=Parameter)
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, Category=Parameter)
	bool bSucceedOnPlay = false;

	TStateTreeExternalDataHandle<class URecallAudioSubsystem> AudioSubsystemHandle;
};
