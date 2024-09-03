// Fill out your copyright notice in the Description page of Project Settings.


#include "UMG_TasksContainer.h"

#include "UMG_BasicEditer.h"
#include "UMG_BasicTask.h"
#include "Blueprint/DragDropOperation.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "MyRewardProject/MyRewardProject.h"
#include "MyRewardProject/GameInstanceSubsystems/MySaveGIS.h"


void UUMG_TasksContainer::TaskFinish(UUMG_BasicTask* Uumg_BasicTask)
{
	if (!ScrollBox_Tasks_Finish->HasChild(Uumg_BasicTask))
	{
		ScrollBox_Tasks_Finish->AddChild(Uumg_BasicTask);
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
	BasicTask->TaskData.SortName = ComboBoxString_TasksClassification->GetSelectedOption();
	BasicTask->TaskData.bIsAddScore = false;

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


bool UUMG_TasksContainer::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                       UDragDropOperation* InOperation)
{
	if (UUMG_BasicTask* OtherBasicTask = Cast<UUMG_BasicTask>(InOperation->Payload))
	{
		/// Selection of ScrollBox
		UScrollBox* SelectedScrollBox = ScrollBox_Tasks;

		// Get the geometry
		FGeometry WidgetGeometry = ScrollBox_Tasks_Finish->GetCachedGeometry();
		FVector2D LocalCoordinate(0, 0);
		FVector2D PixelPosition;
		FVector2D ViewportPosition;
		USlateBlueprintLibrary::LocalToViewport
			(this, WidgetGeometry, LocalCoordinate, PixelPosition, ViewportPosition);

		FVector2D MousePosition(InDragDropEvent.GetScreenSpacePosition());

		//Calc MousePosition(Regardless the screen size, it will always remain the same and correct position)
		FVector2D RealTimeMousePosition = MousePosition - ViewportPosition;

		//Local Size (Right Down Corner)
		FVector2D RightDownCorner = WidgetGeometry.GetLocalSize();

		if (RealTimeMousePosition.X > 0 && RealTimeMousePosition.Y > 0 &&
			RealTimeMousePosition.X < RightDownCorner.X && RealTimeMousePosition.Y < RightDownCorner.Y)
		{
			SelectedScrollBox = ScrollBox_Tasks_Finish;
		}

		//get children position then calculate
		int32 TempIndex = CalcAndGetIndex(MousePosition, SelectedScrollBox);

		//Other operation
		SelectedScrollBox->InsertChildAt(TempIndex, OtherBasicTask);

		SortPanelWidgetsChildren(SelectedScrollBox);

		MySaveGIS->SaveAllData();
	}

	return false;
}

void UUMG_TasksContainer::SortPanelWidgetsChildren(UPanelWidget* InPanelWidget)
{
	TArray<UWidget*> Children = InPanelWidget->GetAllChildren();

	Children.Sort([InPanelWidget](const UWidget& A, const UWidget& B)
	{
		int32 IndexA = InPanelWidget->GetChildIndex(&A);
		int32 IndexB = InPanelWidget->GetChildIndex(&B);
		return IndexA < IndexB;
	});

	InPanelWidget->ClearChildren();
	for (UWidget* Child : Children)
	{
		InPanelWidget->AddChild(Child);
	}
}

void UUMG_TasksContainer::TaskDataAddToTask(FTaskData InTaskData)
{
	//TaskData to BasicUmg
	UUMG_BasicTask* BasicTask = CreateWidget<UUMG_BasicTask>(GetOwningPlayer(), UIClass);
	BasicTask->TaskData = InTaskData;

	//Check AnotherDay
	if (int32 TempAnotherDay = GetWorld()->GetGameInstance()->GetSubsystem<UMySaveGIS>()->AnotherDay)
	{
		MySaveGIS->Global_AllDataToSave.GlobalDailyProgress_Saved = 0;

		BasicTask->TaskData.SavedDays -= TempAnotherDay;
		if (BasicTask->TaskData.SavedDays < 0)
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
		TaskDataAddToTask(InTaskData);
	}

	FNumberFormattingOptions* NumberFormattingOptions = new FNumberFormattingOptions();
	NumberFormattingOptions->SetUseGrouping(false);
	FText TempText = FText::AsNumber(MySaveGIS->Global_AllDataToSave.GlobalTotalScore, NumberFormattingOptions);
	
	//Init UI
	TextBlock_Score->SetText(TempText);
	TextBlock_GlobalDailyProgress_Saved->SetText(
		FText::AsNumber(MySaveGIS->Global_AllDataToSave.GlobalDailyProgress_Saved));
	BasicEditer_GlobalDailyProgress->TextBlock->SetText(
		FText::AsNumber(MySaveGIS->Global_AllDataToSave.GlobalDailyProgress));
	BasicEditer_DailyProgressRewardValue->TextBlock->SetText(
		FText::AsNumber(MySaveGIS->Global_AllDataToSave.DailyProgressRewardValue));


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


bool UUMG_TasksContainer::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                           UDragDropOperation* InOperation)
{
	if (UUMG_BasicTask* OtherBasicTask = Cast<UUMG_BasicTask>(InOperation->Payload))
	{
		/// Selection of ScrollBox
		UScrollBox* SelectedScrollBox = ScrollBox_Tasks;

		// Get the geometry
		FGeometry WidgetGeometry = ScrollBox_Tasks_Finish->GetCachedGeometry();
		FVector2D LocalCoordinate(0, 0);
		FVector2D PixelPosition;
		FVector2D ViewportPosition;
		USlateBlueprintLibrary::LocalToViewport
			(this, WidgetGeometry, LocalCoordinate, PixelPosition, ViewportPosition);

		FVector2D MousePosition(InDragDropEvent.GetScreenSpacePosition());

		//Calc MousePosition(Regardless the screen size, it will always remain the same and correct position)
		FVector2D RealTimeMousePosition = MousePosition - ViewportPosition;

		//Local Size (Right Down Corner)
		FVector2D RightDownCorner = WidgetGeometry.GetLocalSize();

		if (RealTimeMousePosition.X > 0 && RealTimeMousePosition.Y > 0 &&
			RealTimeMousePosition.X < RightDownCorner.X && RealTimeMousePosition.Y < RightDownCorner.Y)
		{
			SelectedScrollBox = ScrollBox_Tasks_Finish;
		}

		//get children position then calculate
		int32 TempIndex = CalcAndGetIndex(MousePosition, SelectedScrollBox);

		//Other operation
		SelectedScrollBox->InsertChildAt(TempIndex, OtherBasicTask);

		SortPanelWidgetsChildren(SelectedScrollBox);

		MySaveGIS->SaveAllData();
	}

	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}


float UUMG_TasksContainer::NumberDeOrIncreaseGradually(float Number, float SavedNumber, float Speed, float LessThan)
{
	if (FMath::Abs(Number - SavedNumber) < LessThan)
	{
		BPDoClearHandle();
		return FMath::RoundToInt32(Speed / 100.f * (SavedNumber > Number ? 1 : -1) + Number);
	}
	else
	{
		return Speed / 100.f * (SavedNumber > Number ? 1 : -1) + Number;
	}
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
