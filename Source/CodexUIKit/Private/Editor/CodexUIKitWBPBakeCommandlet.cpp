#include "Editor/CodexUIKitWBPBakeCommandlet.h"

#if WITH_EDITOR

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "UObject/SavePackage.h"
#include "UI/CodexUIKitControlsPanelWidget.h"
#include "UI/CodexUIKitInventoryGridWidget.h"
#include "UI/CodexUIKitPopupWidget.h"
#include "UI/CodexUIKitQuestBoardWidget.h"
#include "UI/CodexUIKitStandalonePanelWidget.h"
#include "UI/CodexUIKitStartupWidget.h"
#include "UI/CodexUIKitToastWidget.h"
#include "UI/CodexUIStyle.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"

namespace
{
constexpr const TCHAR* WbpPath = TEXT("/Game/UI/WBP");
constexpr const TCHAR* SourceArtPath = TEXT("/Game/UI/SourceArt");

FString ObjectPath(const FString& PackagePath, const FString& AssetName)
{
	return PackagePath / AssetName + TEXT(".") + AssetName;
}

void DeleteAssetIfExists(const FString& PackagePath, const FString& AssetName)
{
	const FString ExistingPath = ObjectPath(PackagePath, AssetName);
	if (UObject* ExistingAsset = StaticLoadObject(UObject::StaticClass(), nullptr, *ExistingPath))
	{
		TArray<UObject*> ObjectsToDelete;
		ObjectsToDelete.Add(ExistingAsset);
		ObjectTools::DeleteObjectsUnchecked(ObjectsToDelete);
	}
}

bool SaveAsset(UObject* Asset)
{
	if (!Asset)
	{
		return false;
	}

	UPackage* Package = Asset->GetOutermost();
	Package->MarkPackageDirty();

	const FString PackageFileName = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;
	return UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs);
}

template <typename TWidget>
TWidget* NewWidget(UWidgetBlueprint* Blueprint, const FName Name, const bool bIsVariable = false)
{
	TWidget* Widget = Blueprint->WidgetTree->ConstructWidget<TWidget>(TWidget::StaticClass(), Name);
	Widget->bIsVariable = bIsVariable;
	Widget->SetFlags(RF_Transactional);
	return Widget;
}

UTextBlock* Text(UWidgetBlueprint* Blueprint, const FName Name, const FText& Value, const float Size, const FLinearColor& Color, const FName Typeface = NAME_None, const bool bIsVariable = false)
{
	UTextBlock* TextBlock = NewWidget<UTextBlock>(Blueprint, Name, bIsVariable);
	TextBlock->SetText(Value);
	TextBlock->SetFont(FCodexUIStyle::Font(Size, Typeface));
	TextBlock->SetColorAndOpacity(FSlateColor(Color));
	return TextBlock;
}

UBorder* Panel(UWidgetBlueprint* Blueprint, const FName Name, const FLinearColor& Fill, const float Radius, const FMargin Padding = FMargin(16.0f))
{
	UBorder* Border = NewWidget<UBorder>(Blueprint, Name);
	Border->SetPadding(Padding);
	Border->SetBrush(FCodexUIStyle::RoundedBrush(Fill, Radius, FCodexUIColor::BorderSoft(), 1.0f));
	return Border;
}

UButton* Button(UWidgetBlueprint* Blueprint, const FName Name, const FText& Label, const ECodexUIButtonKind Kind, const bool bIsVariable = true)
{
	UButton* NewButton = NewWidget<UButton>(Blueprint, Name, bIsVariable);
	NewButton->SetStyle(FCodexUIStyle::ButtonStyle(Kind));

	UTextBlock* LabelText = Text(
		Blueprint,
		*(Name.ToString() + TEXT("_Label")),
		Label,
		FCodexUIFontSize::Body,
		Kind == ECodexUIButtonKind::Neutral ? FCodexUIColor::TextPrimary() : FCodexUIColor::TextInverse(),
		TEXT("Bold"));
	LabelText->SetJustification(ETextJustify::Center);
	NewButton->SetContent(LabelText);
	return NewButton;
}

USlider* Slider(UWidgetBlueprint* Blueprint, const FName Name, const float Value)
{
	USlider* NewSlider = NewWidget<USlider>(Blueprint, Name, true);
	NewSlider->SetValue(Value);
	NewSlider->SetStepSize(0.01f);
	NewSlider->SetSliderBarColor(FCodexUIColor::InfoDark());
	NewSlider->SetSliderHandleColor(FCodexUIColor::Primary());
	return NewSlider;
}

USizeBox* Sized(UWidgetBlueprint* Blueprint, UWidget* Content, const float Width, const float Height, const FName Name = NAME_None)
{
	USizeBox* SizeBox = NewWidget<USizeBox>(Blueprint, Name);
	SizeBox->SetWidthOverride(Width);
	SizeBox->SetHeightOverride(Height);
	SizeBox->AddChild(Content);
	return SizeBox;
}

UCanvasPanelSlot* AddCanvas(UCanvasPanel* Canvas, UWidget* Child, const FVector2D Position, const FVector2D Size, const int32 ZOrder = 0)
{
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
	Slot->SetPosition(Position);
	Slot->SetSize(Size);
	Slot->SetZOrder(ZOrder);
	return Slot;
}

UCanvasPanelSlot* AddCanvasCentered(UCanvasPanel* Canvas, UWidget* Child, const FVector2D Position, const bool bAutoSize = true)
{
	UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Child);
	Slot->SetAnchors(FAnchors(0.5f, 0.5f));
	Slot->SetAlignment(FVector2D(0.5f, 0.5f));
	Slot->SetPosition(Position);
	Slot->SetAutoSize(bAutoSize);
	return Slot;
}

void AddVBox(UVerticalBox* Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), const EHorizontalAlignment HorizontalAlignment = HAlign_Fill)
{
	UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
	Slot->SetPadding(Padding);
	Slot->SetHorizontalAlignment(HorizontalAlignment);
}

void AddHBox(UHorizontalBox* Box, UWidget* Child, const FMargin Padding = FMargin(0.0f), const float FillWidth = 0.0f)
{
	UHorizontalBoxSlot* Slot = Box->AddChildToHorizontalBox(Child);
	Slot->SetPadding(Padding);
	FSlateChildSize Size(FillWidth > 0.0f ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic);
	Size.Value = FillWidth > 0.0f ? FillWidth : 1.0f;
	Slot->SetSize(Size);
}

UWidgetBlueprint* CreateWidgetBlueprint(const FString& AssetName, UClass* ParentClass)
{
	DeleteAssetIfExists(WbpPath, AssetName);

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
	Factory->ParentClass = ParentClass;

	UObject* Asset = AssetToolsModule.Get().CreateAsset(AssetName, WbpPath, UWidgetBlueprint::StaticClass(), Factory);
	UWidgetBlueprint* Blueprint = Cast<UWidgetBlueprint>(Asset);
	if (!Blueprint || !Blueprint->WidgetTree)
	{
		return nullptr;
	}

	Blueprint->Modify();
	Blueprint->WidgetTree->Modify();
	return Blueprint;
}

UTexture2D* ImportTexture(const FString& FileName)
{
	const FString AssetName = FPaths::GetBaseFilename(FileName);
	DeleteAssetIfExists(SourceArtPath, AssetName);

	const FString SourceFile = FPaths::ProjectContentDir() / TEXT("UI/SourceArt") / FileName;
	TArray<uint8> CompressedData;
	if (!FFileHelper::LoadFileToArray(CompressedData, *SourceFile))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load source texture file: %s"), *SourceFile);
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to decode source texture file: %s"), *SourceFile);
		return nullptr;
	}

	TArray64<uint8> RawData;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to read raw texture data: %s"), *SourceFile);
		return nullptr;
	}

	UPackage* Package = CreatePackage(*(FString(SourceArtPath) / AssetName));
	UTexture2D* Texture = NewObject<UTexture2D>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
	Texture->Source.Init(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), 1, 1, TSF_BGRA8, RawData.GetData());
	Texture->SetPlatformData(new FTexturePlatformData());
	Texture->GetPlatformData()->SizeX = ImageWrapper->GetWidth();
	Texture->GetPlatformData()->SizeY = ImageWrapper->GetHeight();
	Texture->GetPlatformData()->PixelFormat = PF_B8G8R8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Texture->GetPlatformData()->Mips.Add(Mip);
	Mip->SizeX = ImageWrapper->GetWidth();
	Mip->SizeY = ImageWrapper->GetHeight();
	void* TextureData = Mip->BulkData.Lock(LOCK_READ_WRITE);
	TextureData = Mip->BulkData.Realloc(RawData.Num());
	FMemory::Memcpy(TextureData, RawData.GetData(), RawData.Num());
	Mip->BulkData.Unlock();

	Texture->SRGB = true;
	Texture->CompressionSettings = TC_EditorIcon;
	Texture->MipGenSettings = TMGS_NoMipmaps;
	Texture->LODGroup = TEXTUREGROUP_UI;
	Texture->NeverStream = true;
	Texture->UpdateResource();

	FAssetRegistryModule::AssetCreated(Texture);
	SaveAsset(Texture);
	return Texture;
}

void FinishBlueprint(UWidgetBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	FAssetRegistryModule::AssetCreated(Blueprint);
	SaveAsset(Blueprint);
}

void SetRoot(UWidgetBlueprint* Blueprint, UWidget* Root)
{
	Blueprint->WidgetTree->RootWidget = Root;
	Root->bIsVariable = false;
	Root->SetFlags(RF_Transactional);
}

void BuildStartup(UWidgetBlueprint* Blueprint)
{
	UCanvasPanel* Root = NewWidget<UCanvasPanel>(Blueprint, TEXT("RootCanvas"));
	SetRoot(Blueprint, Root);

	UBorder* LauncherPanel = Panel(Blueprint, TEXT("LauncherPanel"), FCodexUIColor::SurfaceBase(), FCodexUIRadius::LG, FMargin(20.0f));
	UVerticalBox* Stack = NewWidget<UVerticalBox>(Blueprint, TEXT("LauncherStack"));
	LauncherPanel->SetContent(Stack);

	AddVBox(Stack, Text(Blueprint, TEXT("LauncherTitle"), NSLOCTEXT("CodexUIKitWBP", "LauncherTitle", "Codex UIKit"), FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold")));
	AddVBox(Stack, Text(Blueprint, TEXT("LauncherSubtitle"), NSLOCTEXT("CodexUIKitWBP", "LauncherSubtitle", "테스트할 UI를 선택하세요."), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary()), FMargin(0.0f, 4.0f, 0.0f, 14.0f));

	struct FButtonSpec
	{
		FName Name;
		FText Label;
		ECodexUIButtonKind Kind;
	};

	const FButtonSpec Specs[] =
	{
		{ TEXT("QuestBoardButton"), NSLOCTEXT("CodexUIKitWBP", "QuestBoard", "퀘스트 보드 테스트"), ECodexUIButtonKind::Info },
		{ TEXT("InventoryButton"), NSLOCTEXT("CodexUIKitWBP", "Inventory", "인벤토리 패널 테스트"), ECodexUIButtonKind::Info },
		{ TEXT("ControlsButton"), NSLOCTEXT("CodexUIKitWBP", "Controls", "컨트롤/슬라이더 테스트"), ECodexUIButtonKind::Info },
		{ TEXT("ConfirmPopupButton"), NSLOCTEXT("CodexUIKitWBP", "ConfirmPopup", "확인 팝업 테스트"), ECodexUIButtonKind::Accent },
		{ TEXT("RewardPopupButton"), NSLOCTEXT("CodexUIKitWBP", "RewardPopup", "보상 팝업 테스트"), ECodexUIButtonKind::Info },
		{ TEXT("ToastButton"), NSLOCTEXT("CodexUIKitWBP", "Toast", "토스트 메시지 테스트"), ECodexUIButtonKind::Primary },
		{ TEXT("CloseButton"), NSLOCTEXT("CodexUIKitWBP", "CloseLauncher", "런처 닫기"), ECodexUIButtonKind::Neutral },
	};

	for (const FButtonSpec& Spec : Specs)
	{
		AddVBox(Stack, Sized(Blueprint, Button(Blueprint, Spec.Name, Spec.Label, Spec.Kind), 292.0f, 30.0f), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	AddCanvas(Root, LauncherPanel, FVector2D(32.0f, 32.0f), FVector2D(332.0f, 386.0f));
}

void BuildStandalonePanel(UWidgetBlueprint* Blueprint)
{
	UCanvasPanel* Root = NewWidget<UCanvasPanel>(Blueprint, TEXT("RootCanvas"));
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetRoot(Blueprint, Root);

	UBorder* PanelBorder = Panel(Blueprint, TEXT("StandaloneFrame"), FCodexUIColor::SurfaceBase(), FCodexUIRadius::LG, FMargin(16.0f));
	UVerticalBox* Stack = NewWidget<UVerticalBox>(Blueprint, TEXT("PanelStack"));
	PanelBorder->SetContent(Stack);

	UHorizontalBox* Header = NewWidget<UHorizontalBox>(Blueprint, TEXT("PanelHeader"));
	UTextBlock* Title = Text(Blueprint, TEXT("TitleTextBlock"), NSLOCTEXT("CodexUIKitWBP", "PanelTitle", "패널 테스트"), FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold"), true);
	AddHBox(Header, Title, FMargin(0.0f), 1.0f);
	AddHBox(Header, Sized(Blueprint, Button(Blueprint, TEXT("CloseButton"), NSLOCTEXT("CodexUIKitWBP", "PanelClose", "닫기"), ECodexUIButtonKind::Neutral), 74.0f, 34.0f));
	AddVBox(Stack, Header, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	USizeBox* ContentHost = NewWidget<USizeBox>(Blueprint, TEXT("ContentHost"), true);
	ContentHost->SetWidthOverride(640.0f);
	ContentHost->SetHeightOverride(420.0f);
	AddVBox(Stack, ContentHost);

	AddCanvasCentered(Root, PanelBorder, FVector2D::ZeroVector);
}

void BuildPopup(UWidgetBlueprint* Blueprint)
{
	UOverlay* Root = NewWidget<UOverlay>(Blueprint, TEXT("RootOverlay"));
	SetRoot(Blueprint, Root);

	UBorder* Dimmer = Panel(Blueprint, TEXT("Dimmer"), FLinearColor(0.0f, 0.0f, 0.0f, 0.28f), 0.0f, FMargin(0.0f));
	UOverlaySlot* DimmerSlot = Root->AddChildToOverlay(Dimmer);
	DimmerSlot->SetHorizontalAlignment(HAlign_Fill);
	DimmerSlot->SetVerticalAlignment(VAlign_Fill);

	UBorder* PanelBorder = Panel(Blueprint, TEXT("PopupPanel"), FCodexUIColor::SurfaceBase(), FCodexUIRadius::LG, FMargin(24.0f));
	UVerticalBox* Stack = NewWidget<UVerticalBox>(Blueprint, TEXT("PopupStack"));
	PanelBorder->SetContent(Stack);

	UHorizontalBox* Header = NewWidget<UHorizontalBox>(Blueprint, TEXT("PopupHeader"));
	UTextBlock* Title = Text(Blueprint, TEXT("TitleLabel"), NSLOCTEXT("CodexUIKitWBP", "PopupTitle", "확인"), FCodexUIFontSize::H2, FCodexUIColor::TextPrimary(), TEXT("Bold"), true);
	AddHBox(Header, Title, FMargin(0.0f), 1.0f);
	AddHBox(Header, Sized(Blueprint, Button(Blueprint, TEXT("CloseButton"), FText::FromString(TEXT("X")), ECodexUIButtonKind::Neutral), 34.0f, 30.0f));
	AddVBox(Stack, Header, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UTextBlock* Icon = Text(Blueprint, TEXT("PopupIcon"), FText::FromString(TEXT("!")), 28.0f, FCodexUIColor::Warning(), TEXT("Bold"));
	Icon->SetJustification(ETextJustify::Center);
	AddVBox(Stack, Icon, FMargin(0.0f, 8.0f, 0.0f, 12.0f), HAlign_Center);

	UTextBlock* Message = Text(Blueprint, TEXT("MessageLabel"), NSLOCTEXT("CodexUIKitWBP", "PopupMessage", "정말로 진행하시겠습니까?"), FCodexUIFontSize::Body, FCodexUIColor::TextSecondary(), NAME_None, true);
	Message->SetJustification(ETextJustify::Center);
	Message->SetWrapTextAt(310.0f);
	AddVBox(Stack, Message, FMargin(0.0f, 0.0f, 0.0f, 20.0f));

	UHorizontalBox* Buttons = NewWidget<UHorizontalBox>(Blueprint, TEXT("PopupButtons"));
	UButton* Cancel = Button(Blueprint, TEXT("CancelButton"), NSLOCTEXT("CodexUIKitWBP", "PopupCancel", "취소"), ECodexUIButtonKind::Neutral);
	UTextBlock* CancelLabel = Cast<UTextBlock>(Cancel->GetContent());
	if (CancelLabel)
	{
		CancelLabel->Rename(TEXT("CancelLabel"), nullptr, REN_DontCreateRedirectors);
		CancelLabel->bIsVariable = true;
	}
	UButton* Confirm = Button(Blueprint, TEXT("ConfirmButton"), NSLOCTEXT("CodexUIKitWBP", "PopupConfirm", "확인"), ECodexUIButtonKind::Primary);
	UTextBlock* ConfirmLabel = Cast<UTextBlock>(Confirm->GetContent());
	if (ConfirmLabel)
	{
		ConfirmLabel->Rename(TEXT("ConfirmLabel"), nullptr, REN_DontCreateRedirectors);
		ConfirmLabel->bIsVariable = true;
	}
	AddHBox(Buttons, Sized(Blueprint, Cancel, 146.0f, 38.0f), FMargin(0.0f, 0.0f, 8.0f, 0.0f), 1.0f);
	AddHBox(Buttons, Sized(Blueprint, Confirm, 146.0f, 38.0f), FMargin(8.0f, 0.0f, 0.0f, 0.0f), 1.0f);
	AddVBox(Stack, Buttons);

	USizeBox* PopupSize = Sized(Blueprint, PanelBorder, 380.0f, 0.0f, TEXT("PopupSize"));
	PopupSize->ClearHeightOverride();
	UOverlaySlot* PanelSlot = Root->AddChildToOverlay(PopupSize);
	PanelSlot->SetHorizontalAlignment(HAlign_Center);
	PanelSlot->SetVerticalAlignment(VAlign_Center);
}

void BuildToast(UWidgetBlueprint* Blueprint)
{
	UCanvasPanel* Root = NewWidget<UCanvasPanel>(Blueprint, TEXT("RootCanvas"));
	Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetRoot(Blueprint, Root);

	UBorder* ToastPanel = Panel(Blueprint, TEXT("ToastPanel"), FLinearColor(1.0f, 0.96f, 0.82f, 0.98f), FCodexUIRadius::LG, FMargin(16.0f, 12.0f));
	UHorizontalBox* Row = NewWidget<UHorizontalBox>(Blueprint, TEXT("ToastRow"));
	ToastPanel->SetContent(Row);

	UBorder* Accent = Panel(Blueprint, TEXT("ToastAccent"), FCodexUIColor::Primary(), FCodexUIRadius::SM, FMargin(0.0f));
	AddHBox(Row, Sized(Blueprint, Accent, 5.0f, 56.0f), FMargin(0.0f, 0.0f, 12.0f, 0.0f));

	UVerticalBox* TextStack = NewWidget<UVerticalBox>(Blueprint, TEXT("ToastTextStack"));
	AddVBox(TextStack, Text(Blueprint, TEXT("TitleTextBlock"), NSLOCTEXT("CodexUIKitWBP", "ToastTitle", "토스트 메시지"), FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold"), true));
	UTextBlock* Message = Text(Blueprint, TEXT("MessageTextBlock"), NSLOCTEXT("CodexUIKitWBP", "ToastMessage", "테스트 알림입니다."), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary(), NAME_None, true);
	Message->SetWrapTextAt(250.0f);
	AddVBox(TextStack, Message, FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	AddHBox(Row, TextStack, FMargin(0.0f), 1.0f);

	AddHBox(Row, Sized(Blueprint, Button(Blueprint, TEXT("CloseButton"), FText::FromString(TEXT("x")), ECodexUIButtonKind::Neutral), 34.0f, 30.0f), FMargin(12.0f, 0.0f, 0.0f, 0.0f));

	UCanvasPanelSlot* ToastSlot = AddCanvas(Root, ToastPanel, FVector2D(-32.0f, 32.0f), FVector2D(360.0f, 92.0f));
	ToastSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	ToastSlot->SetAlignment(FVector2D(1.0f, 0.0f));
}

void BuildInventoryGrid(UWidgetBlueprint* Blueprint)
{
	UBorder* RootPanel = Panel(Blueprint, TEXT("InventoryRootPanel"), FCodexUIColor::SurfaceBase(), FCodexUIRadius::LG, FMargin(16.0f));
	SetRoot(Blueprint, RootPanel);

	UVerticalBox* RootStack = NewWidget<UVerticalBox>(Blueprint, TEXT("InventoryRootStack"));
	RootPanel->SetContent(RootStack);

	UHorizontalBox* HeaderRow = NewWidget<UHorizontalBox>(Blueprint, TEXT("InventoryHeaderRow"));
	UTextBlock* HeaderLabel = Text(Blueprint, TEXT("HeaderLabel"), NSLOCTEXT("CodexUIKitWBP", "InventoryHeader", "인벤토리"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold"), true);
	AddHBox(HeaderRow, HeaderLabel, FMargin(0.0f), 1.0f);
	UTextBlock* FilterLabel = Text(Blueprint, TEXT("FilterLabel"), NSLOCTEXT("CodexUIKitWBP", "InventoryFilterDefault", "전체"), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary(), TEXT("Bold"), true);
	AddHBox(HeaderRow, FilterLabel);
	AddVBox(RootStack, HeaderRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UHorizontalBox* FilterRow = NewWidget<UHorizontalBox>(Blueprint, TEXT("InventoryFilterRow"));
	AddHBox(FilterRow, Sized(Blueprint, Button(Blueprint, TEXT("AllFilterButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryFilterAll", "전체"), ECodexUIButtonKind::Primary), 58.0f, 30.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(FilterRow, Sized(Blueprint, Button(Blueprint, TEXT("MaterialFilterButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryFilterMaterial", "재료"), ECodexUIButtonKind::Neutral), 58.0f, 30.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(FilterRow, Sized(Blueprint, Button(Blueprint, TEXT("ConsumableFilterButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryFilterConsumable", "소모품"), ECodexUIButtonKind::Neutral), 68.0f, 30.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(FilterRow, Sized(Blueprint, Button(Blueprint, TEXT("EquipmentFilterButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryFilterEquipment", "장비"), ECodexUIButtonKind::Neutral), 58.0f, 30.0f));
	AddVBox(RootStack, FilterRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UScrollBox* ItemScrollBox = NewWidget<UScrollBox>(Blueprint, TEXT("ItemScrollBox"), true);
	ItemScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	AddVBox(RootStack, Sized(Blueprint, ItemScrollBox, 304.0f, 186.0f), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S3));

	UVerticalBox* DetailBox = NewWidget<UVerticalBox>(Blueprint, TEXT("DetailBox"), true);
	AddVBox(RootStack, Sized(Blueprint, DetailBox, 304.0f, 78.0f), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UHorizontalBox* ButtonRow = NewWidget<UHorizontalBox>(Blueprint, TEXT("InventoryButtonRow"));
	AddHBox(ButtonRow, Sized(Blueprint, Button(Blueprint, TEXT("PreviousItemButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryPrev", "이전"), ECodexUIButtonKind::Neutral), 58.0f, 32.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(ButtonRow, Sized(Blueprint, Button(Blueprint, TEXT("NextItemButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryNext", "다음"), ECodexUIButtonKind::Neutral), 58.0f, 32.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(ButtonRow, Sized(Blueprint, Button(Blueprint, TEXT("UseItemButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryUse", "사용"), ECodexUIButtonKind::Primary), 74.0f, 32.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(ButtonRow, Sized(Blueprint, Button(Blueprint, TEXT("LockItemButton"), NSLOCTEXT("CodexUIKitWBP", "InventoryLock", "잠금"), ECodexUIButtonKind::Info), 74.0f, 32.0f));
	AddVBox(RootStack, ButtonRow);
}

void AddSliderRow(UWidgetBlueprint* Blueprint, UVerticalBox* Parent, const FText& Label, const FName SliderName, const float Value, const FName ValueLabelName, const FMargin Padding)
{
	UHorizontalBox* Row = NewWidget<UHorizontalBox>(Blueprint, FName(*(SliderName.ToString() + TEXT("_Row"))));
	AddHBox(Row, Text(Blueprint, FName(*(SliderName.ToString() + TEXT("_Title"))), Label, FCodexUIFontSize::Body, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f), 0.25f);
	AddHBox(Row, Sized(Blueprint, Slider(Blueprint, SliderName, Value), 280.0f, 26.0f), FMargin(FCodexUISpace::S2, 0.0f), 1.0f);
	UTextBlock* ValueLabel = Text(Blueprint, ValueLabelName, FText::FromString(TEXT("0")), FCodexUIFontSize::Caption, FCodexUIColor::TextSecondary(), TEXT("Bold"), true);
	ValueLabel->SetJustification(ETextJustify::Right);
	AddHBox(Row, Sized(Blueprint, ValueLabel, 46.0f, 22.0f));
	AddVBox(Parent, Row, Padding);
}

void BuildControlsPanel(UWidgetBlueprint* Blueprint)
{
	UBorder* RootPanel = Panel(Blueprint, TEXT("ControlsRootPanel"), FCodexUIColor::SurfaceBase(), FCodexUIRadius::LG, FMargin(16.0f));
	SetRoot(Blueprint, RootPanel);

	UHorizontalBox* RootRow = NewWidget<UHorizontalBox>(Blueprint, TEXT("ControlsRootRow"));
	RootPanel->SetContent(RootRow);

	UVerticalBox* SliderColumn = NewWidget<UVerticalBox>(Blueprint, TEXT("SliderColumn"));
	AddVBox(SliderColumn, Text(Blueprint, TEXT("ControlsTitle"), NSLOCTEXT("CodexUIKitWBP", "ControlsTitle", "컨트롤/슬라이더"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold")), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));
	AddSliderRow(Blueprint, SliderColumn, FText::FromString(TEXT("BGM")), TEXT("BgmSlider"), 0.68f, TEXT("BgmValueLabel"), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S1));
	AddSliderRow(Blueprint, SliderColumn, NSLOCTEXT("CodexUIKitWBP", "SfxSliderLabel", "효과음"), TEXT("SfxSlider"), 0.74f, TEXT("SfxValueLabel"), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S1));
	AddSliderRow(Blueprint, SliderColumn, NSLOCTEXT("CodexUIKitWBP", "ZoomSliderLabel", "줌"), TEXT("ZoomSlider"), 0.38f, TEXT("ZoomValueLabel"), FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UHorizontalBox* ButtonRow = NewWidget<UHorizontalBox>(Blueprint, TEXT("ControlsButtonRow"));
	AddHBox(ButtonRow, Sized(Blueprint, Button(Blueprint, TEXT("ResetButton"), NSLOCTEXT("CodexUIKitWBP", "ControlsReset", "초기화"), ECodexUIButtonKind::Neutral), 82.0f, 32.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S2, 0.0f));
	AddHBox(ButtonRow, Sized(Blueprint, Button(Blueprint, TEXT("AddNoticeButton"), NSLOCTEXT("CodexUIKitWBP", "ControlsAddNotice", "알림 추가"), ECodexUIButtonKind::Primary), 96.0f, 32.0f));
	AddVBox(SliderColumn, ButtonRow);
	AddHBox(RootRow, SliderColumn, FMargin(0.0f, 0.0f, FCodexUISpace::S5, 0.0f), 1.0f);

	UVerticalBox* NoticeColumn = NewWidget<UVerticalBox>(Blueprint, TEXT("NoticeColumn"));
	UHorizontalBox* NoticeHeaderRow = NewWidget<UHorizontalBox>(Blueprint, TEXT("NoticeHeaderRow"));
	UTextBlock* NoticeHeaderLabel = Text(Blueprint, TEXT("NoticeHeaderLabel"), NSLOCTEXT("CodexUIKitWBP", "ControlsNoticeLog", "알림 로그"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold"), true);
	AddHBox(NoticeHeaderRow, NoticeHeaderLabel, FMargin(0.0f), 1.0f);
	AddHBox(NoticeHeaderRow, Sized(Blueprint, Button(Blueprint, TEXT("ScrollTopButton"), NSLOCTEXT("CodexUIKitWBP", "ControlsTop", "위"), ECodexUIButtonKind::Neutral), 44.0f, 30.0f), FMargin(0.0f, 0.0f, FCodexUISpace::S1, 0.0f));
	AddHBox(NoticeHeaderRow, Sized(Blueprint, Button(Blueprint, TEXT("ScrollBottomButton"), NSLOCTEXT("CodexUIKitWBP", "ControlsBottom", "아래"), ECodexUIButtonKind::Neutral), 50.0f, 30.0f));
	AddVBox(NoticeColumn, NoticeHeaderRow, FMargin(0.0f, 0.0f, 0.0f, FCodexUISpace::S2));

	UScrollBox* NoticeScrollBox = NewWidget<UScrollBox>(Blueprint, TEXT("NoticeScrollBox"), true);
	NoticeScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	AddVBox(NoticeColumn, Sized(Blueprint, NoticeScrollBox, 344.0f, 126.0f));
	AddHBox(RootRow, NoticeColumn, FMargin(0.0f), 0.9f);
}

void AddImage(UWidgetBlueprint* Blueprint, UCanvasPanel* Canvas, UTexture2D* Texture, const FName Name, const FVector2D Position, const FVector2D Size, const int32 ZOrder)
{
	UImage* Image = NewWidget<UImage>(Blueprint, Name);
	if (Texture)
	{
		Image->SetBrushFromTexture(Texture, true);
	}
	Image->SetColorAndOpacity(FLinearColor::White);
	AddCanvas(Canvas, Image, Position, Size, ZOrder);
}

void BuildQuestBoard(UWidgetBlueprint* Blueprint, const TMap<FString, UTexture2D*>& Textures)
{
	UCanvasPanel* Root = NewWidget<UCanvasPanel>(Blueprint, TEXT("RootCanvas"));
	SetRoot(Blueprint, Root);

	UBorder* Background = Panel(Blueprint, TEXT("QuestBoardFallbackBackground"), FCodexUIColor::BgMist(), 0.0f, FMargin(0.0f));
	AddCanvas(Root, Background, FVector2D::ZeroVector, FVector2D(1120.0f, 640.0f), -10);

	AddImage(Blueprint, Root, Textures.FindRef(TEXT("quest_header_plate")), TEXT("QuestHeaderPlateImage"), FVector2D(22.0f, 18.0f), FVector2D(1076.0f, 122.0f), -2);
	AddImage(Blueprint, Root, Textures.FindRef(TEXT("quest_side_panel")), TEXT("QuestSidePanelImage"), FVector2D(24.0f, 154.0f), FVector2D(210.0f, 450.0f), -1);
	AddImage(Blueprint, Root, Textures.FindRef(TEXT("quest_list_panel")), TEXT("QuestListPanelImage"), FVector2D(252.0f, 154.0f), FVector2D(350.0f, 450.0f), -1);
	AddImage(Blueprint, Root, Textures.FindRef(TEXT("quest_detail_panel")), TEXT("QuestDetailPanelImage"), FVector2D(620.0f, 154.0f), FVector2D(476.0f, 450.0f), -1);

	AddCanvas(Root, Text(Blueprint, TEXT("QuestGameTitle"), NSLOCTEXT("CodexUIKitWBP", "QuestGameTitle", "TUNA SWEEPER"), FCodexUIFontSize::H3, FCodexUIColor::TextInverse(), TEXT("Bold")), FVector2D(54.0f, 34.0f), FVector2D(260.0f, 32.0f), 1);
	AddCanvas(Root, Text(Blueprint, TEXT("QuestBoardTitle"), NSLOCTEXT("CodexUIKitWBP", "QuestBoardTitle", "QUEST BOARD"), 40.0f, FCodexUIColor::TextInverse(), TEXT("Bold")), FVector2D(54.0f, 64.0f), FVector2D(330.0f, 58.0f), 1);

	UVerticalBox* FilterStack = NewWidget<UVerticalBox>(Blueprint, TEXT("QuestFilterStack"));
	const struct
	{
		FName Name;
		FText Label;
	} FilterSpecs[] =
	{
		{ TEXT("AllFilterButton"), NSLOCTEXT("CodexUIKitWBP", "FilterAll", "전체 퀘스트") },
		{ TEXT("InProgressFilterButton"), NSLOCTEXT("CodexUIKitWBP", "FilterProgress", "진행 중") },
		{ TEXT("ReadyFilterButton"), NSLOCTEXT("CodexUIKitWBP", "FilterReady", "완료 가능") },
		{ TEXT("CompletedFilterButton"), NSLOCTEXT("CodexUIKitWBP", "FilterCompleted", "완료") },
		{ TEXT("FailedFilterButton"), NSLOCTEXT("CodexUIKitWBP", "FilterFailed", "실패") },
	};
	for (const auto& Spec : FilterSpecs)
	{
		AddVBox(FilterStack, Sized(Blueprint, Button(Blueprint, Spec.Name, Spec.Label, ECodexUIButtonKind::Neutral), 166.0f, 36.0f), FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}
	AddCanvas(Root, FilterStack, FVector2D(48.0f, 192.0f), FVector2D(166.0f, 220.0f), 2);

	UTextBlock* HeaderLabel = Text(Blueprint, TEXT("HeaderLabel"), NSLOCTEXT("CodexUIKitWBP", "QuestHeaderLabel", "퀘스트 보드"), FCodexUIFontSize::H3, FCodexUIColor::TextPrimary(), TEXT("Bold"), true);
	HeaderLabel->SetWrapTextAt(300.0f);
	AddCanvas(Root, HeaderLabel, FVector2D(278.0f, 178.0f), FVector2D(300.0f, 54.0f), 2);

	UScrollBox* ScrollBox = NewWidget<UScrollBox>(Blueprint, TEXT("QuestScrollBox"), true);
	ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	AddCanvas(Root, ScrollBox, FVector2D(276.0f, 238.0f), FVector2D(304.0f, 248.0f), 2);

	AddCanvas(Root, Sized(Blueprint, Button(Blueprint, TEXT("PreviousButton"), NSLOCTEXT("CodexUIKitWBP", "PrevQuest", "이전"), ECodexUIButtonKind::Neutral), 94.0f, 34.0f), FVector2D(282.0f, 518.0f), FVector2D(94.0f, 34.0f), 2);
	AddCanvas(Root, Sized(Blueprint, Button(Blueprint, TEXT("NextButton"), NSLOCTEXT("CodexUIKitWBP", "NextQuest", "다음"), ECodexUIButtonKind::Neutral), 94.0f, 34.0f), FVector2D(386.0f, 518.0f), FVector2D(94.0f, 34.0f), 2);

	UVerticalBox* DetailBox = NewWidget<UVerticalBox>(Blueprint, TEXT("DetailBox"), true);
	AddCanvas(Root, DetailBox, FVector2D(652.0f, 192.0f), FVector2D(410.0f, 282.0f), 2);

	AddCanvas(Root, Sized(Blueprint, Button(Blueprint, TEXT("AdvanceObjectiveButton"), NSLOCTEXT("CodexUIKitWBP", "AdvanceObjective", "목표 진행"), ECodexUIButtonKind::Primary), 148.0f, 40.0f), FVector2D(652.0f, 520.0f), FVector2D(148.0f, 40.0f), 2);
	AddCanvas(Root, Sized(Blueprint, Button(Blueprint, TEXT("CycleStateButton"), NSLOCTEXT("CodexUIKitWBP", "CycleState", "상태 순환"), ECodexUIButtonKind::Info), 148.0f, 40.0f), FVector2D(816.0f, 520.0f), FVector2D(148.0f, 40.0f), 2);
}
}

UCodexUIKitWBPBakeCommandlet::UCodexUIKitWBPBakeCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UCodexUIKitWBPBakeCommandlet::Main(const FString& Params)
{
	TMap<FString, UTexture2D*> ImportedTextures;
	const FString TextureNames[] =
	{
		TEXT("quest_header_plate.png"),
		TEXT("quest_side_panel.png"),
		TEXT("quest_list_panel.png"),
		TEXT("quest_detail_panel.png"),
	};

	for (const FString& TextureName : TextureNames)
	{
		UTexture2D* Texture = ImportTexture(TextureName);
		ImportedTextures.Add(FPaths::GetBaseFilename(TextureName), Texture);
	}

	if (UWidgetBlueprint* Startup = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitStartup"), UCodexUIKitStartupWidget::StaticClass()))
	{
		BuildStartup(Startup);
		FinishBlueprint(Startup);
	}
	if (UWidgetBlueprint* Standalone = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitStandalonePanel"), UCodexUIKitStandalonePanelWidget::StaticClass()))
	{
		BuildStandalonePanel(Standalone);
		FinishBlueprint(Standalone);
	}
	if (UWidgetBlueprint* Popup = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitPopup"), UCodexUIKitPopupWidget::StaticClass()))
	{
		BuildPopup(Popup);
		FinishBlueprint(Popup);
	}
	if (UWidgetBlueprint* Toast = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitToast"), UCodexUIKitToastWidget::StaticClass()))
	{
		BuildToast(Toast);
		FinishBlueprint(Toast);
	}
	if (UWidgetBlueprint* InventoryGrid = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitInventoryGrid"), UCodexUIKitInventoryGridWidget::StaticClass()))
	{
		BuildInventoryGrid(InventoryGrid);
		FinishBlueprint(InventoryGrid);
	}
	if (UWidgetBlueprint* ControlsPanel = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitControlsPanel"), UCodexUIKitControlsPanelWidget::StaticClass()))
	{
		BuildControlsPanel(ControlsPanel);
		FinishBlueprint(ControlsPanel);
	}
	if (UWidgetBlueprint* QuestBoard = CreateWidgetBlueprint(TEXT("WBP_CodexUIKitQuestBoard"), UCodexUIKitQuestBoardWidget::StaticClass()))
	{
		BuildQuestBoard(QuestBoard, ImportedTextures);
		FinishBlueprint(QuestBoard);
	}

	return 0;
}

#endif
