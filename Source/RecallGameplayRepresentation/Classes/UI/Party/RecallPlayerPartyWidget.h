// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonUserWidget.h"
#include "Mass/EntityHandle.h"
#include "Observer/Representation/RecallRepresentationReactInterface.h"

#include "RecallPlayerPartyWidget.generated.h"

class URecallPlayerCardWidget;

#define MS_PLAYER_PARTY_CARD_COUNT 8

/**
 * Generic widget for a party of players.
 */
UCLASS(Abstract)
class RECALLGAMEPLAYREPRESENTATION_API URecallPlayerPartyWidget :
	public UExtendedCommonUserWidget,
	public IRecallRepresentationReactInterface
{
	GENERATED_UCLASS_BODY()
	
	// UUserWidget implementation Begin
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// UUserWidget implementation End
	
	// IRecallRepresentationReactInterface implementation begin
protected:
	virtual void OnRender() override;
	// IRecallRepresentationReactInterface implementation end
	
protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_1;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_2;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_3;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_4;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_5;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_6;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_7;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional))
	TObjectPtr<URecallPlayerCardWidget> PlayerCard_8;

	void UpdatePartyCards();

	/**
	 * Called once the party entities are gathered and before they are assigned to the cards.
	 * Override to reorder the entities, only the first MS_PLAYER_PARTY_CARD_COUNT ones are displayed.
	 */
	virtual void SortPartyPlayerEntities(TArray<FMassEntityHandle>& PlayerEntities) const {}

	TObjectPtr<URecallPlayerCardWidget> GetPlayerCard(int32 PlayerIndex) const;
};
