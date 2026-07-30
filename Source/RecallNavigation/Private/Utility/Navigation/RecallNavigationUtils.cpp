// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0


#include "Utility/Navigation/RecallNavigationUtils.h"

#include "Desync/RecallDesyncLog.h"
#include "BaseGeneratedNavLinksProxy.h"
#include "MassExecutionContext.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Simulation/Navigation/RecallPathFollowingFragments.h"
#include "System/Navigation/RecallNavigationTypes.h"

#if RECALL_DESYNC_LOG
#include "Kismet/KismetStringLibrary.h"
#endif // RECALL_DESYNC_LOG

namespace Recall::Navigation::Utils
{

bool StartPathFollowingFromNavMeshPoints(
	const TArray<FRecallNavigationPathPoint>& NavMeshPathPoints,
	FRecallPathFollowerFragment& PathFollowerFragment)
{
	// Convert NavMesh path to generic path points and start following
	PathFollowerFragment.PathPoints.Reset(NavMeshPathPoints.Num());
	for (const FRecallNavigationPathPoint& NavPoint : NavMeshPathPoints)
	{
		// Preserve NavLink flag in the generic path point
		uint8 Flags = 0;
		if (NavPoint.bIsNavLink)
		{
			Flags |= Recall::PathFollowing::PathPointFlag_NavLink;
		}
		FRecallPathPoint PathPoint{NavPoint.Position, Flags, NavPoint.CustomNavLinkId};
		PathFollowerFragment.PathPoints.Add(PathPoint);
	}

	if (PathFollowerFragment.HasPath())
	{
		PathFollowerFragment.Start();
		return true;
	}

	return false;
}

// NOTE: The old utility functions have been deprecated and refactored
// as of the transition to the new generic path following architecture.
// The logic is now handled by specialized processors:
// - URecallNavMeshAgentProcessor: Handles NavMesh path requests and conversion to generic path points
// - URecallPathFollowerProcessor: Handles generic path following with segment dirty flag and NavLink detection
//
// The FollowNavigationPath function is kept for reference and backward compatibility,
// but is no longer used by the main processors since they handle everything inline.
// The segment dirty flag system is now integrated directly into FRecallPathFollowerFragment.

void FollowNavigationPath(const FRecallNavigationContext& NavigationContext)
{
	// DEPRECATED: This function is maintained for backward compatibility only.
	// New code should use URecallPathFollowerProcessor which includes
	// the segment dirty flag and NavLink detection logic.
	// See URecallPathFollowerProcessor::Execute() for the current implementation.

	UE_LOG(LogRecallNavigation, Warning, TEXT("FollowNavigationPath is deprecated. Use URecallPathFollowerProcessor instead."));
}

void RegisterGeneratedLinksProxy(UWorld* World)
{
	/**
	 * This is a hack to make sure that the link proxy is registered.
	 * For clients who join a game, NavSys is not initialized yet when RegisterGeneratedLinksProxy is called.
	 */
	class ARecastNavMeshHack : public ARecastNavMesh
	{
	public:
		void RegisterGeneratedLinksProxy_Exposed()
		{
			if (UWorld* World = GetWorld())
			{
				UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
				if (NavSys)
				{
					for (FNavLinkGenerationJumpConfig& Config : NavLinkJumpConfigs)
					{
						if (!Config.bLinkProxyRegistered && Config.LinkProxy)
						{
							UE_LOG(LogNavLink, Log, TEXT("RegisterLinkProxy id: %llu ptr: 0x%p"), Config.LinkProxyId.GetId(), Config.LinkProxy.Get());
							NavSys->RegisterCustomLink(*Config.LinkProxy);
							Config.bLinkProxyRegistered = true;
						}
					}
				}
			}
		}
	};

	const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (NavSys)
	{
		ARecastNavMesh* Navmesh = NavSys ? Cast<ARecastNavMesh>(NavSys->GetDefaultNavDataInstance()) : nullptr;
		if (Navmesh)
		{
			dynamic_cast<ARecastNavMeshHack*>(Navmesh)->RegisterGeneratedLinksProxy_Exposed();
		}
	}
}


#if RECALL_DESYNC_LOG
/*
void DesyncLogNavigationPath(const FRecallNavigationContext& NavigationContext)
{
	if (!NavigationContext.NavigationFragment.IsMoving())
	{
		return;
	}

	RECALL_DESYNC_LOG_CONTEXT(NavigationContext.ExecutionContext.GetWorld(),
		FString::Printf(TEXT("%s Position: %s, MoveAt: %s"),
		*NavigationContext.Entity.DebugGetDescription(), *NavigationContext.TransformFragment.Position.ToString(),
		*NavigationContext.NavigationFragment.MoveAtLocation.ToString()));

	for (const FRecallNavigationPathPoint& PathPoint : NavigationContext.NavigationFragment.PathPoints)
	{
		RECALL_DESYNC_LOG_CONTEXT(NavigationContext.ExecutionContext.GetWorld(),
			FString::Printf(TEXT("* PathPoint: %s, CustomNavLinkId: %llu, bIsNavLink: %s"),
				*PathPoint.Position.ToString(), PathPoint.CustomNavLinkId.GetId(), *UKismetStringLibrary::Conv_BoolToString(PathPoint.bIsNavLink)));
	}
}
*/
#endif // RECALL_DESYNC_LOG
	
} // namespace Recall::Navigation::Utils
