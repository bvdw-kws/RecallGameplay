// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "RecallGameplayDebugMenuSubsystem.generated.h"

class IDebugMenu;

UCLASS()
class RECALLGAMEPLAYONLINE_API URecallGameplayDebugMenuSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

protected:
	// UWorldSubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// UWorldSubsystem implementation End

	// FTickableGameObject implementation Begin
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// FTickableGameObject implementation End
	
private:
#ifdef WITH_DEBUG_MENU
	TWeakObjectPtr<class UDebugMenuSubsystem> DebugMenuSubsystem;
#endif // WITH_DEBUG_MENU

	void CreateDebugMenuItems(IDebugMenu& DebugMenu);
};
