// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Hw_09 : ModuleRules
{
	public Hw_09(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"UMG","Slate", "SlateCore",
			
			/*
			. SlateCore
			SlateCore는 Slate의 가장 기본적인 기능을 제공합니다.
			위젯의 기본 클래스,레이아웃 시스템,스타일 시스템,브러시(FSlateBrush),
			색상(FSlateColor),입력 이벤트 구조체
			 */
			/*
			2. Slate
			Slate는 실제 UI 위젯들을 제공합니다.
			 */
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Hw_09" });
	}
}
