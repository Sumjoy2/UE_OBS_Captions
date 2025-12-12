// Fill out your copyright notice in the Description page of Project Settings.


#include "OBSCaptionsSubsystem.h"

#include "IWebSocket.h"
#include "JsonObjectConverter.h"
#include "SubtitleManager.h"
#include "WebSocketsModule.h"
#include "Logging/StructuredLog.h"

void UOBSCaptionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWebSocketsModule* WebSocketsModule = &FWebSocketsModule::Get();

    if (FSubtitleManager* Subtitles = FSubtitleManager::GetSubtitleManager())
    {
        Subtitles->OnSetSubtitleText().AddUObject(this, &ThisClass::OnNewSubtitle);
    }
    else
    {
        return;
    }
    
	if (WebSocketsModule != nullptr)
	{
		// Add the connection protocal needed for OBS
		TMap<FString, FString> Headers;
		Headers.Add(TEXT("Sec-WebSocket-Protocol"), TEXT("obswebsocket.json"));

		// TODO: Update to use custom params
		// Initialize connection
		WebSocket = WebSocketsModule->CreateWebSocket(TEXT("ws://127.0.0.1:4455"), FString(TEXT("ws")));
		
		// Setup to process data back
		WebSocket->OnConnected().AddLambda([this]()
		{
			if(IsValid(this) == false) return;
			UE_LOG(LogTemp, Log, TEXT("OnConnected"));
		});

		WebSocket->OnConnectionError().AddLambda([this](const FString& ErrorStr)
		{
			if(IsValid(this) == false) return;
			UE_LOGFMT(LogTemp, Log, "{Msg}", ErrorStr);
		});

		WebSocket->OnMessage().AddUObject(this, &UOBSCaptionsSubsystem::RecievedMessage);

		// Initialize connection to OBS (hopefully)
		WebSocket->Connect();

		// Create json to send
		TSharedPtr<FJsonObject> SendJson = MakeShareable(new FJsonObject());
		// Add op field
		SendJson->SetNumberField(TEXT("op"), 1); // operation one for OSB Identify

		// Create one of our custom structs
		FOBSIdentify Ident;
		Ident.rpcVersion = 1;

		// Turn struct into json
		auto JsonData = FJsonObjectConverter::UStructToJsonObject(Ident);
		// Add struct data to SendJson
		SendJson->SetObjectField(TEXT("d"), JsonData);
		
		// Serialize JSON to string for Sending
		FString Identify;
		TSharedRef<TJsonWriter<>> saveWriter = TJsonWriterFactory<>::Create(&Identify);
		FJsonSerializer::Serialize(SendJson.ToSharedRef(), saveWriter);

		// Send the data to OBS
		WebSocket->Send(Identify);
	}
}

void UOBSCaptionsSubsystem::OnNewSubtitle(const FText& SubtitleText)
{
    if (FSubtitleManager* Subtitles = FSubtitleManager::GetSubtitleManager())
    {
        // Check if the subtitle is actually different to not spam data
        if (LastSubtitleText.EqualTo(SubtitleText))
        {
            return;
        }
        UE_LOGFMT(LogTemp, Log, "{Subtitle}", SubtitleText.ToString());
        if (WebSocket != nullptr)
        {
            // TODO figure out sending Subtitle to OBS
        	// Create json to send
        	TSharedPtr<FJsonObject> SendJson = MakeShareable(new FJsonObject());
        	// Add op field
        	SendJson->SetNumberField(TEXT("op"), 6); // operation six for OBS request

        	FOBSRequest Request;
        	Request.requestType = TEXT("SendStreamCaption");
        	Request.requestId = FGuid::NewGuid().ToString();

        	// Turn struct into json
        	auto JsonData = FJsonObjectConverter::UStructToJsonObject(Request);
        	
        	TSharedPtr<FJsonObject> ReqData = MakeShareable(new FJsonObject());
        	ReqData->SetStringField(TEXT("captionText"), SubtitleText.ToString());
        	
			JsonData->SetObjectField("requestData", ReqData);
        	SendJson->SetObjectField("d", JsonData);

        	FString Caption;
        	TSharedRef<TJsonWriter<>> RequestWritter = TJsonWriterFactory<>::Create(&Caption);
			FJsonSerializer::Serialize(SendJson.ToSharedRef(), RequestWritter);
        	//UE_LOGFMT(LogTemp, Display, "{TheMessage}", Caption);
        	WebSocket->Send(Caption);
        }
        // Update LastSubtitleText to the new text
        LastSubtitleText = SubtitleText;
    }
}

void UOBSCaptionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
	WebSocket->Close();
}

void UOBSCaptionsSubsystem::RecievedMessage(const FString& MessageString)
{
	UE_LOGFMT(LogTemp, Display, "{TheMessage}", MessageString);
}
