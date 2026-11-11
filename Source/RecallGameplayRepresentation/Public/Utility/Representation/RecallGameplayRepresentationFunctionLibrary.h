// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "RecallGameplayRepresentationFunctionLibrary.generated.h"

struct FGameplayTag;
struct FGameplayTagContainer;
struct FMassEntityHandle;
struct FRecallAttributeTrackerRepresentation;
struct FRecallCarryableRepresentation;

UCLASS()
class RECALLGAMEPLAYREPRESENTATION_API URecallGameplayRepresentationFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FString GetLocalPlayerID(const UObject* WorldContextObject, int32 PlayerIndex);
	static FString GetControllerIdByControllerTag(const UObject* WorldContextObject, const FGameplayTag& ControllerTag);
	
	static int32 GetControllerGameplayTagCount(const UObject* WorldContextObject, const FGameplayTag& Tag, const FString& ControllerId);
	static int32 GetGameplayTagCount(const UObject* WorldContextObject, const FGameplayTag& Tag, const FMassEntityHandle& Entity);
	static int32 GetFactionGameplayTagCount(const UObject* WorldContextObject, const FGameplayTagContainer& FactionTags, const FGameplayTag& Tag);
	static FGameplayTagContainer GetGameplayTagsByEntity(const UObject* WorldContextObject, const FMassEntityHandle& Entity);
	static FGameplayTagContainer GetGameplayTagsByControllerID(const UObject* WorldContextObject, const FString& ControllerId);
	static FGameplayTagContainer GetSubGameplayTagsByControllerID(const UObject* WorldContextObject, const FGameplayTag& Tag, const FString& ControllerId);
	static FGameplayTagContainer GetSubGameplayTagsByEntity(const UObject* WorldContextObject, const FGameplayTag& Tag, const FMassEntityHandle& Entity);
	static FGameplayTagContainer GetFactionGameplayTags(const UObject* WorldContextObject, int32 PlayerIndex);
	static FGameplayTagContainer GetFactionGameplayTagsByEntity(const UObject* WorldContextObject, const FMassEntityHandle& Entity);

	static FMassEntityHandle GetPlayerEntity(const UObject* WorldContextObject, const FString& PlayerID);
	static FMassEntityHandle GetEntityByTags(const UObject* WorldContextObject,
		const FGameplayTagContainer& GameplayTags, TArray<FName> NameTags = {});

	static FRecallAttributeTrackerRepresentation GetAttributeTrackerRepresentation(const UObject* WorldContextObject,
		const FMassEntityHandle& Entity);

	static FRecallCarryableRepresentation GetCarryableRepresentation(const UObject* WorldContextObject,
		const FMassEntityHandle& Entity);
};
