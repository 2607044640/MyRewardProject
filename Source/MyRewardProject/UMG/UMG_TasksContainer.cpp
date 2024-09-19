// Fill out your copyright notice in the Description page of Project Settings.


#include "UMG_TasksContainer.h"

#include "BFL_FunctionUtilities.h"
#include "UMG_BasicEditer.h"
#include "UMG_BasicTask.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetArrayLibrary.h"
#include "MyRewardProject/MyRewardProject.h"
#include "MyRewardProject/GameInstanceSubsystems/MySaveGIS.h"


void UUMG_TasksContainer::TaskFinish(UUMG_BasicTask* Uumg_BasicTask)
{
	if (!ScrollBox_Tasks_Finish->HasChild(Uumg_BasicTask))
	{
		ScrollBox_Tasks_Finish->AddChild(Uumg_BasicTask);
	}
}

void UUMG_TasksContainer::RemoveAllSelectedBasicTask()
{
	for (UWidget*
	     Child : ScrollBox_Tasks->GetAllChildren())
	{
		if (UUMG_BasicTask* UMG_BasicTask = Cast<UUMG_BasicTask>(Child))
		{
			if (UMG_BasicTask->OnBasicTaskUnselected.IsBound())
			{
				UMG_BasicTask->OnBasicTaskUnselected.Broadcast();
			}
		}
	}
	for (UWidget*
	     Child : ScrollBox_Tasks_Finish->GetAllChildren())
	{
		if (UUMG_BasicTask* UMG_BasicTask = Cast<UUMG_BasicTask>(Child))
		{
			if (UMG_BasicTask->OnBasicTaskUnselected.IsBound())
			{
				UMG_BasicTask->OnBasicTaskUnselected.Broadcast();
			}
		}
	}

	SelectedBasicTask = nullptr;
}


FReply UUMG_TasksContainer::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	RemoveAllSelectedBasicTask();
	if (TaskContainerOnMouseButtonDown.IsBound())
	{
		TaskContainerOnMouseButtonDown.Broadcast();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUMG_TasksContainer::ScrollTheChildDown(bool IsDown, UWidget* InBasicTask)
{
	if (!InBasicTask)
	{
		return;
	}
	if (UScrollBox* SelectedScrollBox = Cast<UScrollBox>(InBasicTask->GetParent()))
	{
		UBFL_FunctionUtilities::SortPanelWidgetsChildren(SelectedScrollBox);
		//if two scroll box is visible, then set one of them to collapsed
		if (ScrollBox_Tasks->GetVisibility() == ESlateVisibility::Visible &&
			ScrollBox_Tasks_Finish->GetVisibility() == ESlateVisibility::Visible)
		{
			bool bTempCheck = (SelectedScrollBox == ScrollBox_Tasks);
			(bTempCheck ? ScrollBox_Tasks_Finish : ScrollBox_Tasks)->SetVisibility(ESlateVisibility::Collapsed);
		}

		//Other operation
		int32 TempIndex = SelectedScrollBox->GetChildIndex(InBasicTask);
		IsDown ? TempIndex-- : TempIndex++;

		//check if the task is collapsed. when it is,then index++ or --
		//This is to allow users to click only once and the task will be moved, otherwise task may won't move
		while (SelectedScrollBox->GetChildAt(TempIndex))
		{
			if (SelectedScrollBox->GetChildAt(TempIndex)->GetVisibility() == ESlateVisibility::Collapsed)
			{
				IsDown ? TempIndex-- : TempIndex++;
			}
			else
			{
				break;
			}
		}


		// TempIndex = FMath::Clamp(TempIndex, 0, SelectedScrollBox->GetAllChildren().Num()-1);

		MyInsertChildAt(TempIndex, InBasicTask, SelectedScrollBox);

		UBFL_FunctionUtilities::SortPanelWidgetsChildren(SelectedScrollBox);

		MySaveGIS->SaveAllData();


		//set scroll offset
		float ChildrenCount = SelectedScrollBox->GetChildrenCount() - 1;

		float TotalLength = SelectedScrollBox->GetScrollOffsetOfEnd();

		float AverageLength = TotalLength / ChildrenCount;

		float FinalScrollOffset = AverageLength * SelectedScrollBox->GetChildIndex(InBasicTask);

		SelectedScrollBox->SetScrollOffset(FinalScrollOffset);
	}
}

void UUMG_TasksContainer::TaskNotFinish(UUMG_BasicTask* Uumg_BasicTask)
{
	if (!ScrollBox_Tasks->HasChild(Uumg_BasicTask))
	{
		ScrollBox_Tasks->AddChild(Uumg_BasicTask);
	}
}

void UUMG_TasksContainer::ButtonAddTaskOnClick()
{
	UUMG_BasicTask* BasicTask = CreateWidget<UUMG_BasicTask>(GetOwningPlayer(), UIClass);
	ScrollBox_Tasks->AddChild(BasicTask);

	//move new task to the top
	SelectedBasicTask = BasicTask;
	if (UScrollBox* SelectedScrollBox = Cast<UScrollBox>(SelectedBasicTask->GetParent()))
	{
		MyInsertChildAt(0, SelectedBasicTask, SelectedScrollBox);
		UBFL_FunctionUtilities::SortPanelWidgetsChildren(SelectedScrollBox);
		RemoveAllSelectedBasicTask();
	}

	BasicTask->TaskData.SortName = ComboBoxString_TasksClassification->GetSelectedOption();
	BasicTask->TaskData.bIsAddScore = true;
	BasicTask->RefreshUI();

	MySaveGIS->SaveAllData();
}


void UUMG_TasksContainer::ComboBoxString_TasksClassification_OnSelectionChanged(FString SelectedItem,
	ESelectInfo::Type SelectionType)
{
	for (UWidget*
	     Child : ScrollBox_Tasks->GetAllChildren())
	{
		Child->SetVisibility(ESlateVisibility::Collapsed);
		if (UUMG_BasicTask* UMG_BasicTask = Cast<UUMG_BasicTask>(Child))
		{
			if (!SelectedItem.Compare(InitalName_AllTasks))
			{
				Child->SetVisibility(ESlateVisibility::Visible);
			}
			if (!UMG_BasicTask->TaskData.SortName.Compare(SelectedItem))
			{
				Child->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
	for (UWidget*
	     Child : ScrollBox_Tasks_Finish->GetAllChildren())
	{
		Child->SetVisibility(ESlateVisibility::Collapsed);
		if (UUMG_BasicTask* UMG_BasicTask = Cast<UUMG_BasicTask>(Child))
		{
			if (!SelectedItem.Compare(InitalName_AllTasks))
			{
				Child->SetVisibility(ESlateVisibility::Visible);
			}
			if (!UMG_BasicTask->TaskData.SortName.Compare(SelectedItem))
			{
				Child->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}


void UUMG_TasksContainer::TaskDataTransformToTask(FTaskData InTaskData)
{
	//Create widget
	UUMG_BasicTask* BasicTask = CreateWidget<UUMG_BasicTask>(GetOwningPlayer(), UIClass);
	BasicTask->TaskData = InTaskData;

	//Check AnotherDay
	if (int32 TempAnotherDay = GetWorld()->GetGameInstance()->GetSubsystem<UMySaveGIS>()->AnotherDay)
	{
		MySaveGIS->Global_AllDataToSave.GlobalDailyProgress_Saved = 0;

		BasicTask->TaskData.SavedDays -= TempAnotherDay;
		if (BasicTask->TaskData.SavedDays <= 0)
		{
			if (BasicTask->TaskData.Days)
			{
				BasicTask->TaskData.SavedTimes = BasicTask->TaskData.Times;
			}
			BasicTask->TaskData.SavedDays = BasicTask->TaskData.Days;
		}
	}

	if (BasicTask->TaskData.SavedTimes)
	{
		ScrollBox_Tasks->AddChild(BasicTask);
	}
	else
	{
		ScrollBox_Tasks_Finish->AddChild(BasicTask);
	}
}

void UUMG_TasksContainer::BasicEditer_GlobalDailyProgressOnEditFinish(UUMG_BasicTask* Uumg_BasicTask, FText Text)
{
	MySaveGIS->Global_AllDataToSave.GlobalDailyProgress = FCString::Atoi(*Text.ToString());
	BPOnDailyProgressEditFinish();
}

void UUMG_TasksContainer::BasicEditer_DailyProgressRewardValueOnEditFinish(UUMG_BasicTask* Uumg_BasicTask, FText Text)
{
	MySaveGIS->Global_AllDataToSave.DailyProgressRewardValue = FCString::Atoi(*Text.ToString());
}

void UUMG_TasksContainer::NativeConstruct()
{
	Super::NativeConstruct();
	//Init
	TArray<FString> Temp_SortNames;
	Temp_SortNames.Add(InitalName_AllTasks);
	ComboBoxString_TasksClassification->AddOption(InitalName_AllTasks);
	ComboBoxString_TasksClassification->SetSelectedOption(InitalName_AllTasks);

	BasicEditer_GlobalDailyProgress->OnEditFinish.AddUObject(
		this, &UUMG_TasksContainer::BasicEditer_GlobalDailyProgressOnEditFinish);
	BasicEditer_DailyProgressRewardValue->OnEditFinish.AddUObject(
		this, &UUMG_TasksContainer::BasicEditer_DailyProgressRewardValueOnEditFinish);

	//Bind Functions
	ComboBoxString_TasksClassification->OnSelectionChanged.AddDynamic(
		this, &UUMG_TasksContainer::ComboBoxString_TasksClassification_OnSelectionChanged);
	ButtonAddTask->OnReleased.AddDynamic(this, &UUMG_TasksContainer::ButtonAddTaskOnClick);

	//compare and add SortName
	MySaveGIS = GetWorld()->GetGameInstance()->GetSubsystem<UMySaveGIS>();
	for (FTaskData
	     InTaskData : MySaveGIS->Global_AllDataToSave.TaskDatum)
	{
		bool TaskSortNameIsNew = true;
		for (FString
		     SortName : Temp_SortNames)
		{
			if (!InTaskData.SortName.Compare(SortName))
			{
				TaskSortNameIsNew = false;
			}
		}

		if (TaskSortNameIsNew)
		{
			Temp_SortNames.Add(InTaskData.SortName);
			ComboBoxString_TasksClassification->AddOption(InTaskData.SortName);
		}

		//Add tasks
		TaskDataTransformToTask(InTaskData);
	}


	//Init UI
	TextBlock_Score->SetText(
		UBFL_FunctionUtilities::JFFloatToText(MySaveGIS->Global_AllDataToSave.GlobalTotalScore));
	TextBlock_GlobalDailyProgress_Saved->SetText(
		UBFL_FunctionUtilities::JFFloatToText(MySaveGIS->Global_AllDataToSave.GlobalDailyProgress_Saved));
	BasicEditer_GlobalDailyProgress->TextBlock->SetText(
		UBFL_FunctionUtilities::JFFloatToText(MySaveGIS->Global_AllDataToSave.GlobalDailyProgress));
	BasicEditer_DailyProgressRewardValue->TextBlock->SetText(
		UBFL_FunctionUtilities::JFFloatToText(MySaveGIS->Global_AllDataToSave.DailyProgressRewardValue));

	MySaveGIS->SaveAllData();
}

FString UUMG_TasksContainer::FloatToText(float Input)
{
	return FString::SanitizeFloat(Input);
}

float UUMG_TasksContainer::TextBlockTextTofloat(UTextBlock* TextBlock)
{
	return FCString::Atof(*TextBlock->GetText().ToString());
}


UPanelSlot* UUMG_TasksContainer::MyInsertChildAt(int32 Index, UWidget* Content, UPanelWidget* ScrollBox)
{
	UPanelSlot* NewSlot = ScrollBox->AddChild(Content);
	int32 CurrentIndex = ScrollBox->GetChildIndex(Content);
	ScrollBox->Slots.RemoveAt(CurrentIndex);
	ScrollBox->Slots.Insert(Content->Slot, FMath::Clamp(Index, 0, ScrollBox->Slots.Num()));
	return NewSlot;
}


int32 UUMG_TasksContainer::CalcAndGetIndex(FVector2D MousePosition, UPanelWidget* InPanelWidget)
{
	UWidget* SelectChild = nullptr;

	for (UWidget* Child : InPanelWidget->GetAllChildren())
	{
		FGeometry ChildGeometry = Child->GetCachedGeometry();

		// Convert Child's Local Position to Screen/Absolute Position
		FVector2D ChildAbsolutePosition = ChildGeometry.GetAbsolutePosition();
		FVector2D ChildLocalSize = ChildGeometry.GetLocalSize();

		// Adjust Mouse Y Position relative to the middle of the child widget
		float AdjustedMouseY = MousePosition.Y - (ChildLocalSize.Y / 2.0f);

		// Check if the mouse is within the bounds of this child widget
		if (AdjustedMouseY > ChildAbsolutePosition.Y)
		{
			SelectChild = Child;
		}
	}

	// Return the index of the selected child widget or 0 if none found
	if (SelectChild)
	{
		return InPanelWidget->GetChildIndex(SelectChild) + 1;
	}
	return 0;
}
