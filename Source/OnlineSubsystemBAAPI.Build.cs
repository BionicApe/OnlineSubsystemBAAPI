// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class OnlineSubsystemBAAPI : ModuleRules
{
    public OnlineSubsystemBAAPI(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDefinitions.Add("ONLINESUBSYSTEMNULL_PACKAGE=1");
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;


        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "OnlineSubsystem",
                "OnlineSubsystemUtils"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Sockets",
                "Json",
                "BAMultiplayer",
            }
        );
        
        //if (Target.bCompileAgainstEngine)
        //{
        //    PrivateDependencyModuleNames.AddRange(
        //        new string[] {
        //            "Engine"
        //        }
        //    );

        //    PublicDependencyModuleNames.AddRange(
        //       new string[] {
        //            "OnlineSubsystemUtils"
        //       }
        //   );
        //}
    }
}
