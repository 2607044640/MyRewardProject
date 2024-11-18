// Fill out your copyright notice in the Description page of Project Settings.


#include "MySaveGIS.h"

#include "Components/ScrollBox.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "MyRewardProject/MyRewardProject.h"
#include "MyRewardProject/BlueprintFunctionLibraries/BFL_GetClasses.h"
#include "MyRewardProject/Frameworks/MyHUD.h"
#include "MyRewardProject/UMG/UMG_BasicTask.h"
#include "MyRewardProject/UMG/UMG_MainUI.h"
#include "MyRewardProject/UMG/UMG_TasksContainer.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/ScopeLock.h"


bool UMySaveGIS::UploadFileToURLWithAPI(const FString& URL, const FString
                                        & AuthorizationName, const FString& AuthorizationValue,
                                        FString& OutDebugMessage,
                                        const FString& VerbOrMethod, const FString& ContentTypeName,
                                        const FString& ContentTypeValue)
{
	// Validate input parameters
	if (URL.IsEmpty() || AuthorizationName.IsEmpty() || AuthorizationValue.IsEmpty() ||
		VerbOrMethod.IsEmpty() || ContentTypeName.IsEmpty() || ContentTypeValue.IsEmpty())
	{
		OutDebugMessage = TEXT("UploadFileToURLWithAPI: Invalid input parameters");
		UE_LOG(LogTemp, Error, TEXT("UploadFileToURLWithAPI: Invalid input parameters"));
		return false;
	}

	// Ensure the HTTP module is available
	if (!FModuleManager::Get().IsModuleLoaded("HTTP"))
	{
		OutDebugMessage = TEXT("HTTP module not loaded");
		UE_LOG(LogTemp, Error, TEXT("HTTP module not loaded"));
		return false;
	}

	FHttpModule* Http = &FHttpModule::Get();
	if (!Http)
	{
		OutDebugMessage = TEXT("Failed to get HTTP module");
		UE_LOG(LogTemp, Error, TEXT("Failed to get HTTP module"));
		return false;
	}

	// Create your HTTP Request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http->CreateRequest();

	// Set the URL
	Request->SetURL(URL);

	// Set the method to POST
	Request->SetVerb(VerbOrMethod);

	// Set the content type header
	Request->SetHeader(ContentTypeName, ContentTypeValue);
	Request->SetHeader(AuthorizationName, AuthorizationValue);

	// Load the JSON file
	if (!SaveDataFileName.IsEmpty())
	{
		FString FilePath = FPaths::ProjectDir() + TEXT("Saved/MySavedFolder/") + SaveDataFileName;
		if (!FPaths::FileExists(FilePath))
		{
			OutDebugMessage = FString::Printf(TEXT("JSON file does not exist at path: %s"), *FilePath);
			UE_LOG(LogTemp, Error, TEXT("JSON file does not exist at path: %s"), *FilePath);
			return false;
		}

		FString JsonContent;
		if (FFileHelper::LoadFileToString(JsonContent, *FilePath))
		{
			// Set the request content
			Request->SetContentAsString(JsonContent);

			bool bRequestSuccessful = false;

			// Process the response
			Request->OnProcessRequestComplete().BindLambda(
				[&bRequestSuccessful, &OutDebugMessage](FHttpRequestPtr Request, FHttpResponsePtr Response,
				                                        bool bWasSuccessful)
				{
					if (!Request.IsValid() || !Response.IsValid())
					{
						OutDebugMessage = TEXT("Invalid Request or Response");
						UE_LOG(LogTemp, Error, TEXT("Invalid Request or Response"));
						bRequestSuccessful = false;
						return;
					}

					if (bWasSuccessful && Response->GetResponseCode() == 200)
					{
						FString TempStr = FString::Printf(TEXT("Success: %s"), *Response->GetContentAsString());
						OutDebugMessage = TempStr;
						if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TempStr, true, FVector2D(3, 3));
						}
						UE_LOG(LogTemp, Error, TEXT("%s"), *TempStr);
						bRequestSuccessful = true;
					}
					else
					{
						FString TempStr = FString::Printf(TEXT("HTTP Request failed: %s"),
						                                  Response.IsValid()
							                                  ? *Response->GetContentAsString()
							                                  : TEXT("Invalid Response"));
						OutDebugMessage = TempStr;
						if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TempStr, true, FVector2D(3, 3));
						}
						UE_LOG(LogTemp, Error, TEXT("%s"), *TempStr);
						bRequestSuccessful = false;
					}
				});

			// Execute the request
			if (!Request->ProcessRequest())
			{
				OutDebugMessage = TEXT("Failed to process HTTP request");
				UE_LOG(LogTemp, Error, TEXT("Failed to process HTTP request"));
				return false;
			}

			return bRequestSuccessful;
		}
		else
		{
			FString TempStr = FString::Printf(TEXT("Failed to load JSON file"));
			OutDebugMessage = TempStr;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TempStr, true, FVector2D(3, 3));
			}
			UE_LOG(LogTemp, Error, TEXT("%s"), *TempStr);
			return false;
		}
	}
	else
	{
		OutDebugMessage = TEXT("SaveDataFileName is empty");
		UE_LOG(LogTemp, Error, TEXT("SaveDataFileName is empty"));
		return false;
	}
}

void UMySaveGIS::OnHttpRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response->GetResponseCode() == 200)
	{
		FString
			TempStr = FString::Printf(TEXT("Success: %s"), *Response->GetContentAsString());
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TempStr, true, FVector2D(3, 3));
		UE_LOG(LogTemp, Error, TEXT("%s"), *TempStr);
	}
	else
	{
		FString
			TempStr = FString::Printf(TEXT("HTTP Request failed: %s"), *Response->GetContentAsString());
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TempStr, true, FVector2D(3, 3));
		UE_LOG(LogTemp, Error, TEXT("%s"), *TempStr);
	}
}


void UMySaveGIS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadData(Global_AllDataToSave);
	//todo
	// FetchAndParseJSON(TEXT("https://raw.githubusercontent.com/2607044640/NewTestOfHostJson/refs/heads/main/ThisTest.json"));
}


void UMySaveGIS::AddChildrenToBasicDatum(UScrollBox* InScrollBox)
{
	if (!InScrollBox->GetAllChildren().IsValidIndex(0))
	{
		return;
	}
	for (UWidget*
	     Child : InScrollBox->GetAllChildren())
	{
		UUMG_BasicTask* UMG_BasicTask = Cast<UUMG_BasicTask>(Child);
		Global_AllDataToSave.TaskDatum.Add(UMG_BasicTask->TaskData);
	}
}

void UMySaveGIS::SaveAllData()
{
	// Save To AllDataToSave	
	UUMG_TasksContainer* TasksContainer = UBFL_GetClasses::GetMainUI(this)->TasksContainer;
	Global_AllDataToSave.TaskDatum.Empty();
	AddChildrenToBasicDatum(TasksContainer->ScrollBox_Tasks);
	AddChildrenToBasicDatum(TasksContainer->ScrollBox_Tasks_Finish);
	SaveData(Global_AllDataToSave);
}

void UMySaveGIS::AddScore(float AddNum)
{
	Global_AllDataToSave.GlobalTotalScore += AddNum;
	Global_AllDataToSave.GlobalDailyProgress_Saved += AddNum;

	if (Global_AllDataToSave.GlobalDailyProgress <= Global_AllDataToSave.GlobalDailyProgress_Saved &&
		Global_AllDataToSave.GlobalDailyProgress)
	{
		// Calculate the quotient (how many times the divisor fits into the dividend)
		int32 Quotient = Global_AllDataToSave.GlobalDailyProgress_Saved / Global_AllDataToSave.GlobalDailyProgress;

		// Calculate the remainder (what's left after division)
		// int32 Remainder = remainder(Global_AllDataToSave.GlobalDailyProgress_Saved,
		// Global_AllDataToSave.GlobalDailyProgress);

		for (int i = 0; i < Quotient; ++i)
		{
			Global_AllDataToSave.GlobalDailyProgress_Saved -= Global_AllDataToSave.GlobalDailyProgress;
			Global_AllDataToSave.GlobalTotalScore += Global_AllDataToSave.DailyProgressRewardValue;

			//Effect
			AMyHUD* MyHUD = Cast<AMyHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD());
			MyHUD->MainUI->TasksContainer->OnFinishDailyProgress();
		}
	}
}

void UMySaveGIS::MinusScore(float MinusNum)
{
	Global_AllDataToSave.GlobalTotalScore -= MinusNum;
}

float UMySaveGIS::GetScore()
{
	return Global_AllDataToSave.GlobalTotalScore;
}

float UMySaveGIS::GetGlobalDailyProgress_Saved()
{
	return Global_AllDataToSave.GlobalDailyProgress_Saved;
}

float UMySaveGIS::GetDailyProgressRewardValue()
{
	return Global_AllDataToSave.DailyProgressRewardValue;
}


void UMySaveGIS::DelayToGenerateJson()
{
	if (AMyHUD* MyHUD = Cast<AMyHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetHUD()))
	{
		MyHUD->MainUI->TasksContainer->GenerateTasksFromGlobalData();
	}
}

bool UMySaveGIS::SaveData(FAllDataToSave AllDataToSave)
{
	TSharedPtr<FJsonObject> MainJsonObject(new FJsonObject);

	//TaskData
	TArray<TSharedPtr<FJsonValue>> TaskDatumJsonValues;
	for (const FTaskData& TaskData : AllDataToSave.TaskDatum)
	{
		TSharedPtr<FJsonObject> TaskObject(new FJsonObject);

		TaskObject->SetStringField(TEXT("SortName"), TaskData.SortName);
		TaskObject->SetStringField(TEXT("Title"), TaskData.Title);
		TaskObject->SetStringField(TEXT("Detail"), TaskData.Detail);
		TaskObject->SetNumberField(TEXT("Score"), (int32)TaskData.Score);
		TaskObject->SetNumberField(TEXT("Days"), TaskData.Days);
		TaskObject->SetNumberField(TEXT("SavedDays"), TaskData.SavedDays);
		TaskObject->SetNumberField(TEXT("Times"), TaskData.Times);
		TaskObject->SetNumberField(TEXT("SavedTimes"), TaskData.SavedTimes);

		TaskObject->SetBoolField(TEXT("bIsAddScore"), TaskData.bIsAddScore);

		TaskDatumJsonValues.Add(MakeShareable(new FJsonValueObject(TaskObject)));
	}
	MainJsonObject->SetArrayField(TEXT("TaskData"), TaskDatumJsonValues);

	//OtherData
	TArray<TSharedPtr<FJsonValue>> OtherJsonValues;
	TSharedPtr<FJsonObject> OtherJsonObject(new FJsonObject);

	OtherJsonObject->SetNumberField(TEXT("GlobalDayToRecord"), FDateTime::Now().GetDay());
	OtherJsonObject->SetNumberField(TEXT("GlobalTotalScore"), Global_AllDataToSave.GlobalTotalScore);
	OtherJsonObject->SetNumberField(TEXT("GlobalDailyProgress"), Global_AllDataToSave.GlobalDailyProgress);
	OtherJsonObject->SetNumberField(TEXT("GlobalDailyProgress_Saved"), Global_AllDataToSave.GlobalDailyProgress_Saved);
	OtherJsonObject->SetNumberField(TEXT("GlobalDailyProgress"), Global_AllDataToSave.GlobalDailyProgress);
	OtherJsonObject->SetNumberField(TEXT("DailyProgressRewardValue"), Global_AllDataToSave.DailyProgressRewardValue);

	OtherJsonObject->SetStringField(TEXT("URL"), Global_AllDataToSave.URL);
	OtherJsonObject->SetStringField(TEXT("AuthorizationName"), Global_AllDataToSave.AuthorizationName);
	OtherJsonObject->SetStringField(TEXT("AuthorizationValue"), Global_AllDataToSave.AuthorizationValue);
	OtherJsonObject->SetStringField(TEXT("VerbOrMethod"), Global_AllDataToSave.VerbOrMethod);
	OtherJsonObject->SetStringField(TEXT("ContentTypeName"), Global_AllDataToSave.ContentTypeName);
	OtherJsonObject->SetStringField(TEXT("ContentTypeValue"), Global_AllDataToSave.ContentTypeValue);

	OtherJsonValues.Add(MakeShareable(new FJsonValueObject(OtherJsonObject)));

	MainJsonObject->SetArrayField(TEXT("OtherData"), OtherJsonValues);

	// Serialize JSON object to a string
	FString JsonStr;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonStr);

	if (FJsonSerializer::Serialize(MainJsonObject.ToSharedRef(), JsonWriter))
	{
		// Save the JSON string to a file
		FString FilePath = FPaths::ProjectDir() + TEXT("Saved/MySavedFolder/") + SaveDataFileName;

		if (FFileHelper::SaveStringToFile(JsonStr, *FilePath))
		{
			// FString TempStr1 = FString::Printf(TEXT("File saved successfully at %s"), *FilePath);
			// if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TempStr1, true);
			// UE_LOG(LogTemp, Error, TEXT("%s"), *TempStr1);

			return true;
		}
	}
	return false;
}


// size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
// {
// 	userp->append((char*)contents, size * nmemb);
// 	return size * nmemb;
// }
//
// void FetchAndParseJSON(const std::string& url)
// {
// 	CURL* curl = curl_easy_init();
// 	if (curl)
// 	{
// 		std::string readBuffer;
// 		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
// 		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
// 		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
// 		curl_easy_perform(curl);
// 		curl_easy_cleanup(curl);
// 		/*
// 				// Parse JSON
// 				auto jsonData = parse(readBuffer);
// 		
// 				// Example: Access data
// 				for (const auto& task : jsonData["TaskData"])
// 				{
// 					std::string title = task["Title"];
// 					int score = task["Score"];
// 					// Do something with the data...
// 				}
// 				*/
// 	}
// }


void UMySaveGIS::FetchAndParseJSON(const FString& Url)
{
	// Create the HTTP request
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[&](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			// Log the validity of the response
			FString gweg = FString::Printf(
				TEXT("Response.IsValid(): %s"), Response.IsValid() ? TEXT("True") : TEXT("False"));
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, gweg, true, FVector2D(3, 3));
			UE_LOG(LogTemp, Error, TEXT("%s"), *gweg);

			// Log success of the request
			FString oaigw = FString::Printf(TEXT("bWasSuccessful: %s"), bWasSuccessful ? TEXT("True") : TEXT("False"));
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, oaigw, true, FVector2D(3, 3));
			UE_LOG(LogTemp, Error, TEXT("%s"), *oaigw);

			if (bWasSuccessful && Response.IsValid())
			{
				// Get and trim the raw response
				FString RawResponse = Request.Get()->GetResponse()->GetContentAsString();
				UE_LOG(LogTemp, Error, TEXT("Raw Response: %s"), *RawResponse); // Log the raw response

				// Parse JSON
				TSharedRef<TJsonReader<>> SharedRefJson = TJsonReaderFactory<>::Create(RawResponse);
				TSharedPtr<FJsonObject> OutObjectJson;

				if (FJsonSerializer::Deserialize(SharedRefJson, OutObjectJson))
				{
					// Successfully deserialized
					GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("JSON Parsed Successfully"), true,
					                                 FVector2D(3, 3));

					const TArray<TSharedPtr<FJsonValue>>* JsonValues;
					if (OutObjectJson->TryGetArrayField(TEXT("TaskData"), JsonValues))
					{
						for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
						{
							if (TSharedPtr<FJsonObject> TaskObject = JsonValue->AsObject())
							{
								FTaskData TaskData;

								// Use TryGet functions to avoid failures
								TaskObject->TryGetStringField(TEXT("SortName"), TaskData.SortName);
								TaskObject->TryGetStringField(TEXT("Title"), TaskData.Title);
								TaskObject->TryGetStringField(TEXT("Detail"), TaskData.Detail);
								TaskObject->TryGetNumberField(TEXT("Score"), TaskData.Score);
								TaskObject->TryGetNumberField(TEXT("Days"), TaskData.Days);
								TaskObject->TryGetNumberField(TEXT("SavedDays"), TaskData.SavedDays);
								TaskObject->TryGetNumberField(TEXT("Times"), TaskData.Times);
								TaskObject->TryGetNumberField(TEXT("SavedTimes"), TaskData.SavedTimes);
								TaskObject->TryGetBoolField(TEXT("bIsAddScore"), TaskData.bIsAddScore);

								Global_AllDataToSave.TaskDatum.Add(TaskData);
							}
						}
					}
					FTimerHandle TempHandle;
					GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &UMySaveGIS::DelayToGenerateJson, 0.5f);
				}
				else
				{
					// Log error in deserialization
					UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON"));
				}
			}
		});

	// Set up the request
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb("GET");
	HttpRequest->ProcessRequest();
}


bool UMySaveGIS::LoadData(FAllDataToSave& AllDataToSave)
{
	FString FilePath = FPaths::ProjectDir() + TEXT("Saved/MySavedFolder/") + SaveDataFileName;

	FString Result;
	if (FFileHelper::LoadFileToString(Result, *FilePath))
	{
		TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Result);
		TSharedPtr<FJsonObject> JsonObject;

		if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
		{
			// Load task data
			const TArray<TSharedPtr<FJsonValue>>* JsonValues;
			if (JsonObject->TryGetArrayField(TEXT("TaskData"), JsonValues))
			{
				for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
				{
					if (TSharedPtr<FJsonObject> TaskObject = JsonValue->AsObject())
					{
						FTaskData TaskData;
						TaskData.SortName = TaskObject->GetStringField(TEXT("SortName"));
						TaskData.Title = TaskObject->GetStringField(TEXT("Title"));
						TaskData.Detail = TaskObject->GetStringField(TEXT("Detail"));
						TaskData.Score = TaskObject->GetNumberField(TEXT("Score"));
						TaskData.Days = TaskObject->GetNumberField(TEXT("Days"));
						TaskData.SavedDays = TaskObject->GetNumberField(TEXT("SavedDays"));
						TaskData.Times = TaskObject->GetNumberField(TEXT("Times"));
						TaskData.SavedTimes = TaskObject->GetNumberField(TEXT("SavedTimes"));

						TaskData.bIsAddScore = TaskObject->GetBoolField(TEXT("bIsAddScore"));

						AllDataToSave.TaskDatum.Add(TaskData);
					}
				}
			}
			if (JsonObject->TryGetArrayField(TEXT("OtherData"), JsonValues))
			{
				for (const TSharedPtr<FJsonValue>& JsonValue : *JsonValues)
				{
					if (TSharedPtr<FJsonObject> OtherObject = JsonValue->AsObject())
					{
						Global_AllDataToSave.GlobalTotalScore = OtherObject->GetNumberField(
							TEXT("GlobalTotalScore"));
						Global_AllDataToSave.DailyProgressRewardValue = OtherObject->GetNumberField(
							TEXT("DailyProgressRewardValue"));
						Global_AllDataToSave.GlobalDailyProgress = OtherObject->GetNumberField(
							TEXT("GlobalDailyProgress"));
						Global_AllDataToSave.GlobalDailyProgress_Saved = OtherObject->GetNumberField(
							TEXT("GlobalDailyProgress_Saved"));

						Global_AllDataToSave.URL = OtherObject->GetStringField(
							TEXT("URL"));
						Global_AllDataToSave.AuthorizationName = OtherObject->GetStringField(
							TEXT("AuthorizationName"));
						Global_AllDataToSave.AuthorizationValue = OtherObject->GetStringField(
							TEXT("AuthorizationValue"));
						Global_AllDataToSave.VerbOrMethod = OtherObject->GetStringField(
							TEXT("VerbOrMethod"));
						Global_AllDataToSave.ContentTypeName = OtherObject->GetStringField(
							TEXT("ContentTypeName"));
						Global_AllDataToSave.ContentTypeValue = OtherObject->GetStringField(
							TEXT("ContentTypeValue"));

						//AnotherDay calc
						int32 TempDayPrevious = OtherObject->GetNumberField(TEXT("GlobalDayToRecord"));
						int32 TempDayNow = FDateTime::Now().GetDay();
						if (TempDayPrevious != TempDayNow && TempDayNow - TempDayPrevious > 0)
						{
							AnotherDay = TempDayNow - TempDayPrevious;
						}
					}
				}
			}

			return true;
		}
	}
	return false;
}

/*
{
	"TaskData": [
		{
			"SortName": "Null",
			"Title": "oijjjj",
			"Detail": "",
			"Score": 1,
			"Days": 0,
			"SavedDays": 0,
			"Times": 1,
			"SavedTimes": 1
		},
		{
			"SortName": "Null",
			"Title": "aaaaaa",
			"Detail": "",
			"Score": 1,
			"Days": 0,
			"SavedDays": 0,
			"Times": 1,
			"SavedTimes": 0
		},
		{
			"SortName": "Null",
			"Title": "333ffff32fffffffffffffof",
			"Detail": "",
			"Score": 1,
			"Days": 2,
			"SavedDays": 0,
			"Times": 1,
			"SavedTimes": 0
		}
	],
	"OtherData": [
		{
			"GlobalTotalScore": 146,
			"GlobalDailyProgress": 0,
			"GlobalDayToRecord": 21
		}
	]
}
*/
