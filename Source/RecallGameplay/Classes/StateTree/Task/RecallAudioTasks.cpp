// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallAudioTasks.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "System/Audio/RecallAudioSubsystem.h"

bool FRecallPlaySoundTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(AudioSubsystemHandle);
	return true;
}

EStateTreeRunStatus FRecallPlaySoundTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Sound.IsNull())
	{
		return EStateTreeRunStatus::Failed;
	}

	URecallAudioSubsystem& AudioSubsystem = Context.GetExternalData(AudioSubsystemHandle);
	AudioSubsystem.PlaySoundAsync(Sound);
	
	if (bSucceedOnPlay)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	
	return Super::EnterState(Context, Transition);
}
