// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "StateTree/Player/RecallPlayerConditionBase.h"
#include "Engine/DataTable.h"

#include "RecallPlayerConditions.generated.h"

USTRUCT()
struct RECALLGAMEPLAY_API FRecallStateTreeWasInputJustPressedConditionInstanceData
{
	GENERATED_BODY()
};
UE_STATETREE_ZEROED_TRIVIALLY_COPIED_NO_DESTRUCTOR_INSTANCEDATA(FRecallStateTreeWasInputJustPressedConditionInstanceData);

USTRUCT(DisplayName="Was Input Pressed")
struct RECALLGAMEPLAY_API FRecallStateTreeWasInputJustPressedCondition : public FRecallPlayerConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallStateTreeWasInputJustPressedConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;

	UPROPERTY(EditAnywhere, Category="Parameter", meta=(RowType="/Script/RecallCore.RecallInputActionTableRow"))
	FDataTableRowHandle InputAction;
	
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=1))
	int32 InputBufferDuration = 1;

protected:
	TStateTreeExternalDataHandle<struct FRecallPlayerInputFragment> InputFragmentHandle;
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallStateTreeWasAnyInputJustPressedConditionInstanceData
{
	GENERATED_BODY()
};
UE_STATETREE_ZEROED_TRIVIALLY_COPIED_NO_DESTRUCTOR_INSTANCEDATA(FRecallStateTreeWasAnyInputJustPressedConditionInstanceData);

USTRUCT(DisplayName="Was Any Input Pressed")
struct RECALLGAMEPLAY_API FRecallStateTreeWasAnyInputJustPressedCondition : public FRecallPlayerConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallStateTreeWasAnyInputJustPressedConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;

	UPROPERTY(EditAnywhere, Category="Parameter", meta=(ClampMin=1))
	int32 InputBufferDuration = 1;

protected:
	TStateTreeExternalDataHandle<struct FRecallPlayerInputFragment> InputFragmentHandle;
};

USTRUCT()
struct RECALLGAMEPLAY_API FRecallStateTreeIsInputHeldConditionInstanceData
{
	GENERATED_BODY()
};
UE_STATETREE_ZEROED_TRIVIALLY_COPIED_NO_DESTRUCTOR_INSTANCEDATA(FRecallStateTreeIsInputHeldConditionInstanceData);

USTRUCT(DisplayName="Is Input Held")
struct RECALLGAMEPLAY_API FRecallStateTreeIsInputHeldCondition : public FRecallPlayerConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FRecallStateTreeIsInputHeldConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category="Parameter")
	bool bInvert = false;
	
	UPROPERTY(EditAnywhere, Category="Parameter", meta=(RowType="/Script/RecallCore.RecallInputActionTableRow"))
	FDataTableRowHandle InputAction;

protected:
	TStateTreeExternalDataHandle<struct FRecallPlayerInputFragment> InputFragmentHandle;
};
