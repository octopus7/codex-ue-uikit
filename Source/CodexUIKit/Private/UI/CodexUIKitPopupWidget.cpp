#include "UI/CodexUIKitPopupWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/CodexUIStyle.h"

void UCodexUIKitPopupWidget::ConfigurePopup(
	const FText& InTitle,
	const FText& InMessage,
	const FText& InConfirmText,
	const FText& InCancelText,
	bool bInShowCancel)
{
	TitleText = InTitle;
	MessageText = InMessage;
	ConfirmText = InConfirmText;
	CancelText = InCancelText;
	bShowCancel = bInShowCancel;
	SyncText();
}

void UCodexUIKitPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}
}

void UCodexUIKitPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FCodexUIStyle::ApplyFontToTextBlocks(GetRootWidget());
	SyncText();
}

void UCodexUIKitPopupWidget::SyncText()
{
	if (TitleLabel)
	{
		TitleLabel->SetText(TitleText);
	}
	if (MessageLabel)
	{
		MessageLabel->SetText(MessageText);
	}
	if (ConfirmLabel)
	{
		ConfirmLabel->SetText(ConfirmText);
	}
	if (CancelLabel)
	{
		CancelLabel->SetText(CancelText);
	}
	if (CancelButton)
	{
		CancelButton->SetVisibility(bShowCancel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCodexUIKitPopupWidget::HandleCloseClicked()
{
	RemoveFromParent();
}
