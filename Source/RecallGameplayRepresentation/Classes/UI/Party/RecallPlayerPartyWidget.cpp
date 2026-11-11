// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallPlayerPartyWidget.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Mass/EntityHandle.h"
#include "RecallFrontendUtils.h"
#include "RecallPlayerCardWidget.h"
#include "Player/Interface/RecallPlayerStateInterface.h"
#include "Utility/Player/RecallPlayerUtils.h"

URecallPlayerPartyWidget::URecallPlayerPartyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URecallPlayerPartyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Recall::Frontend::Utils::RegisterGlobalObserver<IRecallRepresentationReactInterface>(this);
}

void URecallPlayerPartyWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	Recall::Frontend::Utils::UnregisterAllGlobalObservers(this);
}

void URecallPlayerPartyWidget::OnRender()
{
	UpdatePartyCards();
}

void URecallPlayerPartyWidget::UpdatePartyCards()
{
	const AGameStateBase* GameState = UGameplayStatics::GetGameState(this);
	if (!IsValid(GameState))
	{
		return;
	}

	TArray<FMassEntityHandle> PlayerEntities;

	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		const IRecallPlayerStateInterface* PlayerInterface = Cast<IRecallPlayerStateInterface>(PlayerState);	
		if (!PlayerInterface)
		{
			continue;
		}

		const FString& PlayerId = PlayerInterface->GetSimPlayerId();

		FMassEntityHandle PlayerEntity;
		if (!Recall::Player::Utils::FindPlayerEntityInWorld(GetWorld(),
			PlayerId, PlayerEntity))
		{
			continue;
		}

		PlayerEntities.Add(PlayerEntity);
	}

	SortPartyPlayerEntities(PlayerEntities);

	// More players than cards: only the first entities (after sorting) are displayed.
	int32 PlayerCardIndex = 0;
	for (const FMassEntityHandle& PlayerEntity : PlayerEntities)
	{
		if (PlayerCardIndex >= MS_PLAYER_PARTY_CARD_COUNT)
		{
			break;
		}

		const TObjectPtr<URecallPlayerCardWidget> PlayerCard = GetPlayerCard(
			PlayerCardIndex++);
		if (!PlayerCard)
		{
			continue;
		}

		PlayerCard->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		PlayerCard->SetPlayerEntity(PlayerEntity);
	}

	for (; PlayerCardIndex < MS_PLAYER_PARTY_CARD_COUNT; PlayerCardIndex++)
	{
		const TObjectPtr<URecallPlayerCardWidget> PlayerCard = GetPlayerCard(
			PlayerCardIndex);
		if (PlayerCard)
		{
			PlayerCard->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

TObjectPtr<URecallPlayerCardWidget> URecallPlayerPartyWidget::GetPlayerCard(
	int32 PlayerIndex) const
{
	switch (PlayerIndex)
	{
	case 0:
		return PlayerCard_1;
		
	case 1:
		return PlayerCard_2;
		
	case 2:
		return PlayerCard_3;
		
	case 3:
		return PlayerCard_4;
		
	case 4:
		return PlayerCard_5;
		
	case 5:
		return PlayerCard_6;
		
	case 6:
		return PlayerCard_7;
		
	case 7:
		return PlayerCard_8;

	default:
		unimplemented();
		return nullptr;
	}
}
