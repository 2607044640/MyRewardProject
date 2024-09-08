// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyRewardProject/GameInstanceSubsystems/MySaveGIS.h"
#include "UMG_TasksContainer.generated.h"

class UImage;
class UComboBoxString;
class UUMG_BasicEditer;
class UScrollBox;
class UUMG_BasicTask;
class UButton;
class UTextBlock;
/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(TaskStateChanged1)

UCLASS()
class MYREWARDPROJECT_API UUMG_TasksContainer : public UUserWidget
{
	
	// UPROPERTY(BlueprintAssignable)
	TaskStateChanged1 OnDro1p;
	
	UFUNCTION()
	void ButtonAddTaskOnClick();
	UFUNCTION()
	void ComboBoxString_TasksClassification_OnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	void TaskDataAddToTask(FTaskData InTaskData);
	void BasicEditer_GlobalDailyProgressOnEditFinish(UUMG_BasicTask* Uumg_BasicTask, FText Text);
	void BasicEditer_DailyProgressRewardValueOnEditFinish(UUMG_BasicTask* Uumg_BasicTask, FText Text);

	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void BPOnFinishDailyProgress();

	UFUNCTION(BlueprintCallable)
	FString FloatToText(float Input);
	UFUNCTION(BlueprintCallable)
	float TextBlockTextTofloat(UTextBlock* TextBlock);

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                              UDragDropOperation* InOperation) override;
	UPanelSlot* MyInsertChildAt(int32 Index, UWidget* Content, UScrollBox* ScrollBox);

	int32 CalcAndGetIndex(FVector2D MousePosition, UPanelWidget* InPanelWidget);
	void SortPanelWidgetsChildren(UPanelWidget* InPanelWidget);

	void TaskNotFinish(UUMG_BasicTask* Uumg_BasicTask);
	void TaskFinish(UUMG_BasicTask* Uumg_BasicTask);

	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UComboBoxString* ComboBoxString_TasksClassification;

	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* TextBlock_Score;

	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UButton* Button_AddSortName;
	UPROPERTY(meta = (BindWidget), BlueprintReadWrite)
	UScrollBox* ScrollBox_Tasks;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UScrollBox* ScrollBox_Tasks_Finish;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UUMG_BasicEditer* BasicEditer_GlobalDailyProgress;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UUMG_BasicEditer* BasicEditer_DailyProgressRewardValue;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* TextBlock_GlobalDailyProgress_Saved;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UButton* ButtonAddTask;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JFSetting)
	TSubclassOf<UUMG_BasicTask> UIClass;
	UPROPERTY()
	UMySaveGIS* MySaveGIS;
	double ScrollBoxOffset_Finish;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JFSetting)
	double SpeedOfScroll = 150;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JFSetting)
	double OffsetOfScroll = 110;


	UPROPERTY()
	UScrollBox* SelectedScrollBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=JFSetting)
	bool bCanScroll;
	
	bool bIsScrollUp;
	double SpeedOfScroll_ByEdgeDistance=1;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	GENERATED_BODY()
};
