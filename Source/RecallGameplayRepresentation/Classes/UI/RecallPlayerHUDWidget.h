// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "ExtendedCommonActivatableWidget.h"
#include "Representation/Conversation/RecallConversationReactInterface.h"

#include "RecallPlayerHUDWidget.generated.h"

/**
 * HUD widget for the player.
 */
UCLASS(Abstract)
class RECALLGAMEPLAYREPRESENTATION_API URecallPlayerHUDWidget :
	public UExtendedCommonActivatableWidget,
	public IRecallConversationReactInterface
{
	GENERATED_UCLASS_BODY()

	// UUserWidget implementation Begin
public:
	virtual void OnConversationUpdatedEvent(const FClientConversationMessagePayload& Message, int32 PlayerIndex = 0) override;
	virtual void OnConversationStatusChangedEvent(bool bStarted, int32 PlayerIndex = 0) override;
	// UUserWidget implementation End

	// UUserWidget implementation Begin
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// UUserWidget implementation End
	
protected:
	/**
	 * Conversation widget.
	 * It must be activated from this widget because it is an activable widget.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=HUD)
	TSoftClassPtr<class URecallConversationWidget> ConversationWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly)
	TWeakObjectPtr<class URecallConversationWidget> ConversationWidget;
	
	UPROPERTY(Transient, BlueprintReadOnly)
	uint8 bStartedConversation : 1;
};
