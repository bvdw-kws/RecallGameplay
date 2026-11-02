// Copyright (C) 2024 Van de Walle Bastien
// Licensed under the Apache License, Version 2.0

using UnrealBuildTool;

public class RecallGameplayCueModule : ModuleRules
{
	public RecallGameplayCueModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Engine",
			"GameplayTags",
			"MassCore",
			"RecallCore",
			"RecallSimulation",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"GameplayAbilities",
			"MassEntity",
			"RecallGameplay",
		});
	}
}
