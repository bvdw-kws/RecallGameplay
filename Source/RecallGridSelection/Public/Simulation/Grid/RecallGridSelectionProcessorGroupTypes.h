// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "CoreMinimal.h"

namespace Recall::GridSelection::ProcessorGroupNames
{

	namespace PrePhysics
	{
		/**
		 * Group during which the grid selection cell is set from the player input, before the state trees update.
		 * Processors that need to constrain the selection should run after this group.
		 */
		const FName CursorInput = FName(TEXT("RecallGridSelectionCursorInput"));
	} // namespace PrePhysics

} // namespace Recall::GridSelection::ProcessorGroupNames
