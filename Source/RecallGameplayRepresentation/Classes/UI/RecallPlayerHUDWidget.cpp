// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallPlayerHUDWidget.h"

#include "ExtendedPrimaryGameLayoutTypes.h"
#include "Conversation/RecallConversationWidget.h"
#include "RecallFrontendUtils.h"
#include "PrimaryGameLayout.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

URecallPlayerHUDWidget::URecallPlayerHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputConfig = EExtendedCommonInputMode::Game;
	bStartedConversation = false;
}

void URecallPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Recall::Frontend::Utils::RegisterGlobalObserver<IRecallConversationReactInterface>(this);
}

void URecallPlayerHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Recall::Frontend::Utils::UnregisterAllGlobalObservers(this);
}

void URecallPlayerHUDWidget::OnConversationUpdatedEvent(const FClientConversationMessagePayload& Message,
	int32 PlayerIndex)
{
	if (!IsOwningLocalPlayer(PlayerIndex) || bStartedConversation)
	{
		return;
	}

	bStartedConversation = true;
	
	if (ConversationWidget.IsValid() || ConversationWidgetClass.IsNull())
	{
		return;
	}

	if (UPrimaryGameLayout* RootLayout = UPrimaryGameLayout::GetPrimaryGameLayout(GetOwningPlayer()))
	{
		RootLayout->PushWidgetToLayerStackAsync<URecallConversationWidget>(TAG_UI_LAYER_GAMELAYER, true, ConversationWidgetClass,
			[this, Message](EAsyncWidgetLayerState State, URecallConversationWidget* Widget) {
				ConversationWidget = Widget;
				if (ConversationWidget.IsValid() && State == EAsyncWidgetLayerState::Initialize)
				{
					ConversationWidget->SetConversationStatus(true);
					ConversationWidget->SetConversationMessage(Message);
				}
		});
	}
}

void URecallPlayerHUDWidget::OnConversationStatusChangedEvent(bool bStarted, int32 PlayerIndex)
{
	if (!IsOwningLocalPlayer(PlayerIndex))
	{
		return;
	}

	if (!bStarted && bStartedConversation)
	{
		bStartedConversation = false;
		
		if (ConversationWidget.IsValid())
		{
			ConversationWidget->DeactivateWidget();
			ConversationWidget.Reset();
		}
	}
}
