#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitDemoWidget.generated.h"

class UCanvasPanel;
class UTexture2D;

UCLASS()
class CODEXUIKIT_API UCodexUIKitDemoWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> LoadedSourceTextures;

	void BuildHeader(UCanvasPanel& Canvas);
	void BuildButtons(UCanvasPanel& Canvas);
	void BuildIconAndStatus(UCanvasPanel& Canvas);
	void BuildSideMenu(UCanvasPanel& Canvas);
	void BuildMainPanels(UCanvasPanel& Canvas);
	void BuildBottomExamples(UCanvasPanel& Canvas);

	UTexture2D* LoadSourceTexture(const FString& FileName);
	void ShowPopup(const FText& Title, const FText& Message, const FText& Confirm, const FText& Cancel, bool bShowCancel);

	UFUNCTION()
	void HandleOpenConfirmPopup();

	UFUNCTION()
	void HandleOpenRewardPopup();

	UFUNCTION()
	void HandleOpenActionPopup();
};
