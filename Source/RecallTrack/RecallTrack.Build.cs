// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

using System;
using System.IO;
using UnrealBuildTool;

public class RecallTrack : ModuleRules
{
	public RecallTrack(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Engine",
			"JoltPhysicsRuntimeCore",
			"JoltPhysicsRuntime",
		});
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"MassCore",
			"MassEntity",
			"MassSpawner",
			"RecallCore",
			"RecallSimulation",
			"RecallPhysicsModule",
			"RecallSignals",
			"Landscape",
			"Foliage",
		});
		
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {"LandscapeEditor", "UnrealEd" });
		}
	}
}
