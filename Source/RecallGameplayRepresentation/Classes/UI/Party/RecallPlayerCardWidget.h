// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonUserWidget.h"

#include "RecallPlayerCardWidget.generated.h"

struct FMassEntityHandle;

/**
 * Generic widget for the card of an active player.
 */
UCLASS(Abstract)
class RECALLGAMEPLAYREPRESENTATION_API URecallPlayerCardWidget :
	public UExtendedCommonUserWidget
{
	GENERATED_UCLASS_BODY()

public:
	virtual void SetPlayerEntity(const FMassEntityHandle& Entity);

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<class URecallAttributeGroupWidget> AttributeGroup_Player;
	
};
