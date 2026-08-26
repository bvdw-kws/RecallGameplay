#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"

#include "RecallEquipmentHolderActorInterface.generated.h"

/**
 * Interface for an actor that can carry equipment.
 */
UINTERFACE()
class RECALLINVENTORYMODULE_API URecallEquipmentHolderActorInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class RECALLINVENTORYMODULE_API IRecallEquipmentHolderActorInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	USceneComponent* GetEquipmentSlotComponent(UPARAM(meta=(GameplayTagFilter="EquipSlot")) FGameplayTag EquipSlot) const;
};
