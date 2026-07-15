// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallInteractWidget.h"

#include "CommonActionWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "RecallFrontendUtils.h"
#include "RecallInteractEventWidget.h"
#include "Components/Overlay.h"
#include "Input/CommonUIInputTypes.h"
#include "Player/Interface/RecallPlayerControllerInterface.h"

URecallInteractWidget::URecallInteractWidget()
	: Super()
{
	bDisplayInActionBar = true;
}

void URecallInteractWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Overlay_Interact)
	{
		CanvasPanelSlot = Cast<UCanvasPanelSlot>(Overlay_Interact->Slot);
		if (CanvasPanelSlot)
		{
			CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasPanelSlot->SetPosition(FVector2D(0.f, 0.f));
			CanvasPanelSlot->SetAnchors(FAnchors());
		}
	}

	SetVisibility(ESlateVisibility::Collapsed);
	
	Recall::Frontend::Utils::RegisterGlobalObserver<IRecallInteractReactInterface>(this);
}

void URecallInteractWidget::NativeDestruct()
{
	Super::NativeDestruct();

	Recall::Frontend::Utils::UnregisterAllGlobalObservers(this);
}

void URecallInteractWidget::SetInteraction(const FRecallInteractState& State, int32 PlayerIndex /*= INDEX_NONE*/)
{
	if (GetOwningLocalPlayerIndex() != PlayerIndex)
	{
		return;
	}
	
	if (State.IsValid())
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SetContextual(State.Progress, State.Contextual);
		SetInteractable(State.Progress, State.Interactable, State.InteractableLocation);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}	
}

void URecallInteractWidget::SetContextual(const FRecallInteractProgressRepresentation& Progress,
	const FRecallInteractionRepresentation& Representation)
{
	TSet<TObjectPtr<UInputAction>> ContextualActions;
	
	for (int32 InputIndex = 0; InputIndex < RECALL_INTERACT_INPUT_COUNT; InputIndex++)
	{
		const ERecallInteractInput Input = static_cast<ERecallInteractInput>(InputIndex);

		// No event for this contextual input, skip
		const FRecallInteractEventState* EventStatePtr = Representation.EventMap.Find(Input);
		if (EventStatePtr == nullptr || !EventStatePtr->InputAction)
		{
			continue;
		}

		ContextualActions.Add(EventStatePtr->InputAction);
		
		// Already registered
		{
			const FUIActionBindingHandle* Handle = ContextualActionBindingHandles.Find(EventStatePtr->InputAction);
			if (Handle != nullptr)
			{
				continue;
			}
		}

		const FSimpleDelegate OnExecuteAction = GetOnExecuteContextualActionDelegate(Input);
		
		FBindUIActionArgs BindActionArgs(EventStatePtr->InputAction, true, OnExecuteAction);
		BindActionArgs.InputMode = ECommonInputMode::Game;
		BindActionArgs.bIsPersistent = true;
		BindActionArgs.bConsumeInput = false;
		
		const FUIActionBindingHandle NewHandle = UCommonUserWidget::RegisterUIActionBinding(BindActionArgs);
		ContextualActionBindingHandles.Add(EventStatePtr->InputAction, NewHandle);
	}

	TArray<TObjectPtr<UInputAction>> OldContextualActions;
	ContextualActionBindingHandles.GenerateKeyArray(OldContextualActions);

	// Remove deprecated action bindings
	for (const TObjectPtr<UInputAction>& ContextualAction : OldContextualActions)
	{
		if (ContextualActions.Contains(ContextualAction))
		{
			continue;
		}

		FUIActionBindingHandle DeprecatedHandle;
		ContextualActionBindingHandles.RemoveAndCopyValue(ContextualAction, DeprecatedHandle);
		RemoveActionBinding(DeprecatedHandle);
		DeprecatedHandle.Unregister();
	}
}
FSimpleDelegate URecallInteractWidget::GetOnExecuteContextualActionDelegate(ERecallInteractInput Input)
{	
	switch (Input)
	{
	case ERecallInteractInput::Primary:
		return FSimpleDelegate::CreateUObject(this, &ThisClass::OnExecuteContextualPrimary);
			
	case ERecallInteractInput::Secondary:
		return FSimpleDelegate::CreateUObject(this, &ThisClass::OnExecuteContextualSecondary);
			
	case ERecallInteractInput::Tertiary:
		return FSimpleDelegate::CreateUObject(this, &ThisClass::OnExecuteContextualTertiary);
			
	case ERecallInteractInput::Quaternary:
		return FSimpleDelegate::CreateUObject(this, &ThisClass::OnExecuteContextualQuaternary);

	default:
		unimplemented();
		return FSimpleDelegate();
	}
}

void URecallInteractWidget::OnExecuteContextualPrimary()
{
	PushInputOption(Recall::Input::Option::InteractContextualPrimary);
}

void URecallInteractWidget::OnExecuteContextualSecondary()
{
	PushInputOption(Recall::Input::Option::InteractContextualSecondary);
}

void URecallInteractWidget::OnExecuteContextualTertiary()
{
	PushInputOption(Recall::Input::Option::InteractContextualTertiary);
}

void URecallInteractWidget::OnExecuteContextualQuaternary()
{
	PushInputOption(Recall::Input::Option::InteractContextualQuaternary);
}

void URecallInteractWidget::PushInputOption(const FString& Option)
{
	const TScriptInterface<IRecallPlayerControllerInterface> PlayerController(GetOwningPlayer());
	if (PlayerController)
	{
		PlayerController->SetInputOptions(FString::Printf(TEXT("?%s"), *Option));
	}
}

void URecallInteractWidget::SetInteractable(
	const FRecallInteractProgressRepresentation& Progress, const FRecallInteractionRepresentation& Representation,
	const FVector& Location)
{
	if (ProgressBar_Interact)
	{
		const bool bShowProgress = !Progress.bIsContextual && Progress.IsInProgress() && !Progress.bHideProgress;
		
		ProgressBar_Interact->SetPercent(Progress.Progress);
		ProgressBar_Interact->SetVisibility(bShowProgress ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	for (int32 InputIndex = 0; InputIndex < RECALL_INTERACT_INPUT_COUNT; InputIndex++)
	{
		const ERecallInteractInput Input = static_cast<ERecallInteractInput>(InputIndex);
		const TObjectPtr<URecallInteractEventWidget> EventWidget = GetInteractEventWidget(Input);
		if (!EventWidget)
		{
			continue;
		}

		const FRecallInteractEventState* EventStatePtr = Representation.EventMap.Find(Input);
		if (EventStatePtr != nullptr && !Progress.IsInProgress())
		{
			EventWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			EventWidget->SetInteractEvent(EventStatePtr->Text, EventStatePtr->bFailed);
		}
		else
		{
			EventWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), Location, ScreenPosition, true);

	FVector2D ViewportPosition = FVector2D::ZeroVector;
	USlateBlueprintLibrary::ScreenToViewport(this, ScreenPosition, ViewportPosition);

	if (CanvasPanelSlot)
	{
		CanvasPanelSlot->SetPosition(ViewportPosition);
	}
}

TObjectPtr<class URecallInteractEventWidget> URecallInteractWidget::GetInteractEventWidget(
	ERecallInteractInput Input) const
{
	switch (Input)
	{
	case ERecallInteractInput::Primary:
		return InteractEvent_Primary;
		
	case ERecallInteractInput::Secondary:
		return InteractEvent_Secondary;
		
	case ERecallInteractInput::Tertiary:
		return InteractEvent_Tertiary;
		
	case ERecallInteractInput::Quaternary:
		return InteractEvent_Quaternary;

	case ERecallInteractInput::MAX:
		checkNoEntry();
		return nullptr;

	default:
		unimplemented();
		return nullptr;
	}
}
