// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Public/Subsystems/GameInstanceSubsystem.h"
#include "WebSocketsModule.h"
#include "Templates/SharedPointer.h"
#include "Dom/JsonObject.h"
#include "OBSCaptionsSubsystem.generated.h"

class IWebSocket;

USTRUCT()
struct FOBSIdentify
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 rpcVersion;
};

USTRUCT()
struct FOBSRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	FString requestType;
	UPROPERTY()
	FString requestId;
};

/**
 * 
 */
UCLASS()
class OBSCAPTIONS_API UOBSCaptionsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    void OnNewSubtitle(const FText& SubtitleText); 
    virtual void Deinitialize() override;

protected:
    TSharedPtr<IWebSocket> WebSocket;
    void RecievedMessage(const FString& MessageString);
    
private:
    FText LastSubtitleText;
};
