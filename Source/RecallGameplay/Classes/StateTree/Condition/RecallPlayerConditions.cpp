// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallPlayerConditions.h"

#include "Data/Input/RecallInputActionTableRow.h"
#include "Simulation/Player/Input/RecallPlayerInputFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

//----------------------------------------------------------------------//
// FRecallStateTreeWasInputJustPressedCondition
//----------------------------------------------------------------------//
bool FRecallStateTreeWasInputJustPressedCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(InputFragmentHandle);
	return true;
}

bool FRecallStateTreeWasInputJustPressedCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const ERecallControllerInputCommand ControllerInputCommand = FRecallInputActionTableRow::GetControllerInputCommandByAction(InputAction);

	const FRecallPlayerInputFragment& InputFragment = Context.GetExternalData(InputFragmentHandle);
	const bool bWasInputPressed = ControllerInputCommand != ERecallControllerInputCommand::None && InputFragment.WasInputPressed(ControllerInputCommand, InputBufferDuration);
	return bWasInputPressed != bInvert;
}

//----------------------------------------------------------------------//
// FRecallStateTreeWasAnyInputJustPressedCondition
//----------------------------------------------------------------------//
bool FRecallStateTreeWasAnyInputJustPressedCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(InputFragmentHandle);
	return true;
}

bool FRecallStateTreeWasAnyInputJustPressedCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FRecallPlayerInputFragment& InputFragment = Context.GetExternalData(InputFragmentHandle);
	const bool bWasAnyInputPressed = InputFragment.IsAnyInputPressed(InputBufferDuration);
	return bWasAnyInputPressed != bInvert;
}

//----------------------------------------------------------------------//
// FRecallStateTreeIsInputHeldCondition
//----------------------------------------------------------------------//
bool FRecallStateTreeIsInputHeldCondition::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(InputFragmentHandle);
	return true;
}

bool FRecallStateTreeIsInputHeldCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const ERecallControllerInputCommand ControllerInputCommand = FRecallInputActionTableRow::GetControllerInputCommandByAction(InputAction);

	const FRecallPlayerInputFragment& InputFragment = Context.GetExternalData(InputFragmentHandle);
	const bool bIsInputHeld = ControllerInputCommand != ERecallControllerInputCommand::None && InputFragment.IsInputHeld(ControllerInputCommand);
	return bIsInputHeld != bInvert;
}
