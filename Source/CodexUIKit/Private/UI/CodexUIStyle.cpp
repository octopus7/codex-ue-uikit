#include "UI/CodexUIStyle.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Styling/CoreStyle.h"

FLinearColor FCodexUIColor::BgTealDeep() { return FCodexUIStyle::FromHex(TEXT("#1E6F68")); }
FLinearColor FCodexUIColor::BgTeal() { return FCodexUIStyle::FromHex(TEXT("#3D998E")); }
FLinearColor FCodexUIColor::BgMist() { return FCodexUIStyle::FromHex(TEXT("#EAF2D7")); }
FLinearColor FCodexUIColor::SurfaceBase() { return FCodexUIStyle::FromHex(TEXT("#FFF4DB")); }
FLinearColor FCodexUIColor::SurfaceRaised() { return FCodexUIStyle::FromHex(TEXT("#FFF8E8")); }
FLinearColor FCodexUIColor::SurfaceSunken() { return FCodexUIStyle::FromHex(TEXT("#F2E6C9")); }
FLinearColor FCodexUIColor::BorderSoft() { return FCodexUIStyle::FromHex(TEXT("#DED3B8")); }
FLinearColor FCodexUIColor::BorderStrong() { return FCodexUIStyle::FromHex(TEXT("#C8B995")); }
FLinearColor FCodexUIColor::TextPrimary() { return FCodexUIStyle::FromHex(TEXT("#2F3935")); }
FLinearColor FCodexUIColor::TextSecondary() { return FCodexUIStyle::FromHex(TEXT("#686456")); }
FLinearColor FCodexUIColor::TextMuted() { return FCodexUIStyle::FromHex(TEXT("#918A78")); }
FLinearColor FCodexUIColor::TextInverse() { return FLinearColor::White; }
FLinearColor FCodexUIColor::Primary() { return FCodexUIStyle::FromHex(TEXT("#6DBE7F")); }
FLinearColor FCodexUIColor::PrimaryHover() { return FCodexUIStyle::FromHex(TEXT("#8ED195")); }
FLinearColor FCodexUIColor::PrimaryPressed() { return FCodexUIStyle::FromHex(TEXT("#4FA866")); }
FLinearColor FCodexUIColor::Accent() { return FCodexUIStyle::FromHex(TEXT("#FFB17A")); }
FLinearColor FCodexUIColor::AccentHover() { return FCodexUIStyle::FromHex(TEXT("#FFC48F")); }
FLinearColor FCodexUIColor::Info() { return FCodexUIStyle::FromHex(TEXT("#58BFD6")); }
FLinearColor FCodexUIColor::InfoDark() { return FCodexUIStyle::FromHex(TEXT("#3498D4")); }
FLinearColor FCodexUIColor::Warning() { return FCodexUIStyle::FromHex(TEXT("#F2A02F")); }
FLinearColor FCodexUIColor::Danger() { return FCodexUIStyle::FromHex(TEXT("#E27C70")); }
FLinearColor FCodexUIColor::NewTag() { return FCodexUIStyle::FromHex(TEXT("#7F91C9")); }
FLinearColor FCodexUIColor::Disabled() { return FCodexUIStyle::FromHex(TEXT("#E9E2D2")); }
FLinearColor FCodexUIColor::Coin() { return FCodexUIStyle::FromHex(TEXT("#F7B72E")); }
FLinearColor FCodexUIColor::Gem() { return FCodexUIStyle::FromHex(TEXT("#7567C8")); }

FLinearColor FCodexUIStyle::FromHex(const TCHAR* Hex)
{
	return FLinearColor::FromSRGBColor(FColor::FromHex(Hex));
}

FSlateBrush FCodexUIStyle::RoundedBrush(
	const FLinearColor& Fill,
	float Radius,
	const FLinearColor& Outline,
	float OutlineWidth)
{
	return FSlateRoundedBoxBrush(FSlateColor(Fill), Radius, FSlateColor(Outline), OutlineWidth);
}

FButtonStyle FCodexUIStyle::ButtonStyle(ECodexUIButtonKind Kind)
{
	FLinearColor Normal = FCodexUIColor::SurfaceRaised();
	FLinearColor Hovered = FCodexUIColor::SurfaceBase();
	FLinearColor Pressed = FCodexUIColor::SurfaceSunken();
	FLinearColor Outline = FCodexUIColor::BorderSoft();

	switch (Kind)
	{
	case ECodexUIButtonKind::Primary:
		Normal = FCodexUIColor::Primary();
		Hovered = FCodexUIColor::PrimaryHover();
		Pressed = FCodexUIColor::PrimaryPressed();
		Outline = FCodexUIColor::PrimaryPressed();
		break;
	case ECodexUIButtonKind::Accent:
		Normal = FCodexUIColor::Accent();
		Hovered = FCodexUIColor::AccentHover();
		Pressed = FCodexUIColor::Warning();
		Outline = FCodexUIColor::Warning();
		break;
	case ECodexUIButtonKind::Info:
		Normal = FCodexUIColor::InfoDark();
		Hovered = FCodexUIColor::Info();
		Pressed = FCodexUIStyle::FromHex(TEXT("#247DB4"));
		Outline = FCodexUIStyle::FromHex(TEXT("#247DB4"));
		break;
	case ECodexUIButtonKind::Danger:
		Normal = FCodexUIColor::Danger();
		Hovered = FCodexUIStyle::FromHex(TEXT("#EC9086"));
		Pressed = FCodexUIStyle::FromHex(TEXT("#C95F55"));
		Outline = FCodexUIStyle::FromHex(TEXT("#C95F55"));
		break;
	case ECodexUIButtonKind::Disabled:
		Normal = FCodexUIColor::Disabled();
		Hovered = FCodexUIColor::Disabled();
		Pressed = FCodexUIColor::Disabled();
		Outline = FCodexUIColor::BorderSoft();
		break;
	case ECodexUIButtonKind::Neutral:
	default:
		break;
	}

	FButtonStyle Style;
	Style.SetNormal(RoundedBrush(Normal, FCodexUIRadius::MD, Outline, 1.0f));
	Style.SetHovered(RoundedBrush(Hovered, FCodexUIRadius::MD, Outline, 1.0f));
	Style.SetPressed(RoundedBrush(Pressed, FCodexUIRadius::MD, Outline, 1.0f));
	Style.SetDisabled(RoundedBrush(FCodexUIColor::Disabled(), FCodexUIRadius::MD, FCodexUIColor::BorderSoft(), 1.0f));
	Style.SetNormalPadding(FMargin(0.0f));
	Style.SetPressedPadding(FMargin(1.0f, 1.0f, 0.0f, 0.0f));
	return Style;
}

FSlateFontInfo FCodexUIStyle::Font(float Size, FName Typeface)
{
	const FName ResolvedTypeface = Typeface.IsNone() ? TEXT("Regular") : Typeface;
	return FCoreStyle::GetDefaultFontStyle(ResolvedTypeface, Size);
}

FProgressBarStyle FCodexUIStyle::ProgressBarStyle(const FLinearColor& Fill)
{
	FProgressBarStyle Style;
	Style.SetBackgroundImage(RoundedBrush(FCodexUIStyle::FromHex(TEXT("#D8D2BE")), FCodexUIRadius::SM));
	Style.SetFillImage(RoundedBrush(Fill, FCodexUIRadius::SM));
	return Style;
}
