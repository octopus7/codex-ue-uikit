#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitStandalonePanelWidget.generated.h"

class USizeBox;
class UButton;
class UTextBlock;

UCLASS()
class CODEXUIKIT_API UCodexUIKitStandalonePanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ConfigurePanel(FText InTitle, TSubclassOf<UUserWidget> InContentWidgetClass, FVector2D InContentSize);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleTextBlock;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> ContentHost;

	UPROPERTY()
	FText PanelTitle;

	UPROPERTY()
	TSubclassOf<UUserWidget> ContentWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> ContentWidget;

	FVector2D ContentSize = FVector2D(640.0f, 420.0f);

	void RebuildContent();
	void SyncPanel();

	UFUNCTION()
	void HandleCloseClicked();
};
