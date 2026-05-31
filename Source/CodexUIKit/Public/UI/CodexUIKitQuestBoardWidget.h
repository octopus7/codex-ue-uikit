#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CodexUIKitQuestBoardWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UVerticalBox;
class UButton;

UENUM()
enum class ECodexQuestState : uint8
{
	InProgress,
	ReadyToComplete,
	Completed,
	Failed
};

UENUM()
enum class ECodexQuestFilter : uint8
{
	All,
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
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> QuestScrollBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> DetailBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HeaderLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> PreviousButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> NextButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AdvanceObjectiveButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CycleStateButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> AllFilterButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> InProgressFilterButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReadyFilterButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> CompletedFilterButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> FailedFilterButton;

	UPROPERTY()
	TArray<FCodexQuestDemoData> Quests;

	int32 SelectedQuestIndex = 0;
	ECodexQuestFilter ActiveFilter = ECodexQuestFilter::All;

	void SeedQuestsIfNeeded();
	void RefreshQuestList();
	void RefreshDetail();
	void RefreshFilterButtonStyles();
	void SelectNextQuest(int32 Direction);
	void SetFilter(ECodexQuestFilter NewFilter);
	bool MatchesActiveFilter(const FCodexQuestDemoData& Quest) const;
	bool EnsureSelectedQuestMatchesFilter();
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

	UFUNCTION()
	void HandleAllFilter();

	UFUNCTION()
	void HandleInProgressFilter();

	UFUNCTION()
	void HandleReadyFilter();

	UFUNCTION()
	void HandleCompletedFilter();

	UFUNCTION()
	void HandleFailedFilter();
};
