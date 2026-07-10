// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0

using System;
using System.IO;
using System.Linq;
using UnrealBuildTool;

public class RecallGameplay : ModuleRules
{
	public RecallGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Engine", "VariableCollectionModule" });
		PrivateDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"JoltPhysicsRuntime",
			"MassCore",
			"MassEntity",
			"MassSpawner",
			"RecallCore",
			"RecallSimulation",
			"RecallPhysicsModule",
			"RecallFrontend",
			"RecallSignals",
			"RecallGameplayCore",
			"RecallAttributeModule",
			"StateTreeModule",
			"Niagara",
			"LevelSequence",
			"MovieScene",
			"GameplayTags",
			"AIModule",
			"PropertyBindingUtils",
		});
		
		if (RecallCore.IsPluginEnabled(Target, "MultiWorld"))
		{
			PrivateDependencyModuleNames.Add("MultiWorld");
		}

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "UMG" });
	}
}