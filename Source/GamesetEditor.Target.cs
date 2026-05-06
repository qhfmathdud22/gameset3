using UnrealBuildTool;

public class GamesetEditorTarget : TargetRules
{
	public GamesetEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		UndefinedIdentifierWarningLevel = WarningLevel.Error;
		ExtraModuleNames.Add("Gameset");
	}
}
