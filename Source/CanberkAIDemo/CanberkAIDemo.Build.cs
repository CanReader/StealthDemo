// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CanberkAIDemo : ModuleRules
{
	public CanberkAIDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","AnimGraphRuntime"
, "AssetRegistry", "AIModule", "GameplayTasks", "RenderCore" });
	}
}
