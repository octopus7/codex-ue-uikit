#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitQuestBoardWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UVerticalBox;

UENUM()
enum class ECodexQuestState : uint8
{
	InProgress,
	ReadyToComplete,
	Completed,
	Failed
};

USTRUCT()
struct FCodexQuestDemoData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Title;

	UPROPERTY()
	FString Summary;

	UPROPERTY()
	FString Objective;

	UPROPERTY()
	int32 Current = 0;

	UPROPERTY()
	int32 Required = 1;

	UPROPERTY()
	ECodexQuestState State = ECodexQuestState::InProgress;

	UPROPERTY()
	int32 RewardCoins = 0;
};

UCLASS()
class CODEXUIKIT_API UCodexUIKitQuestBoardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex UIKit")
	void ResetDemoQuests();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<UScrollBox> QuestScrollBox;

	UPROPERTY()
	TObjectPtr<UVerticalBox> DetailBox;

	UPROPERTY()
	TObjectPtr<UTextBlock> HeaderLabel;

	UPROPERTY()
	TArray<FCodexQuestDemoData> Quests;

	int32 SelectedQuestIndex = 0;

	void SeedQuestsIfNeeded();
	void RefreshQuestList();
	void RefreshDetail();
	void SelectNextQuest(int32 Direction);
	static FText StateText(ECodexQuestState State);
	static FLinearColor StateColor(ECodexQuestState State);

	UFUNCTION()
	void HandlePreviousQuest();

	UFUNCTION()
	void HandleNextQuest();

	UFUNCTION()
	void HandleAdvanceObjective();

	UFUNCTION()
	void HandleCycleState();
};
