// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "RecallGameplayDebugMenuSubsystem.h"

#ifdef WITH_DEBUG_MENU
#include "Debug/DebugMenuInterface.h"
#include "System/Debug/DebugMenuSubsystem.h"
#endif // WITH_DEBUG_MENU

void URecallGameplayDebugMenuSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#ifdef WITH_DEBUG_MENU
	Collection.InitializeDependency<UDebugMenuSubsystem>();
	DebugMenuSubsystem = UGameInstance::GetSubsystem<UDebugMenuSubsystem>(GetGameInstance());
	if (DebugMenuSubsystem.IsValid())
	{
		CreateDebugMenuItems(DebugMenuSubsystem->GetMutableDebugMenu());
	}
#endif // WITH_DEBUG_MENU
}

void URecallGameplayDebugMenuSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
#ifdef WITH_DEBUG_MENU
	DebugMenuSubsystem.Reset();
#endif // WITH_DEBUG_MENU
}

void URecallGameplayDebugMenuSubsystem::Tick(float DeltaTime)
{
}

TStatId URecallGameplayDebugMenuSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URecallGameplayDebugMenuContent, STATGROUP_Tickables);
}

void URecallGameplayDebugMenuSubsystem::CreateDebugMenuItems(IDebugMenu& DebugMenu)
{
#ifdef WITH_DEBUG_MENU
	// AI
	{
		DebugMenu.AddItem_Bool(TEXT("AI"), "Show Env Query Result", false, TEXT("recall.stateTree.ShowEnvQueryResult"));
		DebugMenu.AddItem_Bool(TEXT("AI"), "Show Navigation Path", false, TEXT("recall.stateTree.ShowNavigationPath"));
	}
#endif // WITH_DEBUG_MENU
}
