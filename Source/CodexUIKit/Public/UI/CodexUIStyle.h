#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

enum class ECodexUIButtonKind : uint8
{
	Primary,
	Accent,
	Info,
	Danger,
	Neutral,
	Disabled
};

struct FCodexUIColor
{
	static FLinearColor BgTealDeep();
	static FLinearColor BgTeal();
	static FLinearColor BgMist();
	static FLinearColor SurfaceBase();
	static FLinearColor SurfaceRaised();
	static FLinearColor SurfaceSunken();
	static FLinearColor BorderSoft();
	static FLinearColor BorderStrong();
	static FLinearColor TextPrimary();
	static FLinearColor TextSecondary();
	static FLinearColor TextMuted();
	static FLinearColor TextInverse();
	static FLinearColor Primary();
	static FLinearColor PrimaryHover();
	static FLinearColor PrimaryPressed();
	static FLinearColor Accent();
	static FLinearColor AccentHover();
	static FLinearColor Info();
	static FLinearColor InfoDark();
	static FLinearColor Warning();
	static FLinearColor Danger();
	static FLinearColor NewTag();
	static FLinearColor Disabled();
	static FLinearColor Coin();
	static FLinearColor Gem();
};

struct FCodexUISpace
{
	static constexpr float S1 = 4.0f;
	static constexpr float S2 = 8.0f;
	static constexpr float S3 = 12.0f;
	static constexpr float S4 = 16.0f;
	static constexpr float S5 = 20.0f;
	static constexpr float S6 = 24.0f;
	static constexpr float S8 = 32.0f;
	static constexpr float S10 = 40.0f;
};

struct FCodexUIRadius
{
	static constexpr float XS = 4.0f;
	static constexpr float SM = 6.0f;
	static constexpr float MD = 8.0f;
	static constexpr float LG = 12.0f;
	static constexpr float XL = 16.0f;
};

struct FCodexUISize
{
	static constexpr float ButtonSM = 32.0f;
	static constexpr float ButtonMD = 44.0f;
	static constexpr float ButtonLG = 56.0f;
	static constexpr float InventoryCell = 64.0f;
	static constexpr float HotbarCell = 58.0f;
};

struct FCodexUIFontSize
{
	static constexpr float Display = 48.0f;
	static constexpr float H1 = 30.0f;
	static constexpr float H2 = 22.0f;
	static constexpr float H3 = 17.0f;
	static constexpr float Body = 14.0f;
	static constexpr float Caption = 12.0f;
	static constexpr float Badge = 13.0f;
};

struct FCodexUIStyle
{
	static FLinearColor FromHex(const TCHAR* Hex);
	static FSlateBrush RoundedBrush(
		const FLinearColor& Fill,
		float Radius = FCodexUIRadius::MD,
		const FLinearColor& Outline = FLinearColor::Transparent,
		float OutlineWidth = 0.0f);
	static FButtonStyle ButtonStyle(ECodexUIButtonKind Kind);
	static FSlateFontInfo Font(float Size, FName Typeface = NAME_None);
	static FProgressBarStyle ProgressBarStyle(const FLinearColor& Fill);
};
