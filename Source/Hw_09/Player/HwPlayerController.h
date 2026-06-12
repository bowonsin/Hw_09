// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HwPlayerController.generated.h"
class UHwChatInput;
UCLASS()
class HW_09_API AHwPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	
public:
	AHwPlayerController();

	virtual void BeginPlay() override;

	// 채팅값 받아오기
	void SetChatMessageString(const FString& InChatMessageString);
	
	// 받아온 채팅값을 screen으로 출력
	void PrintChatMessageString(const FString& InChatMessageString);
	
	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);
	
	// Timer UI 업데이트 
	UFUNCTION(Client, Reliable)
	void ClientRPCTimerUiUpdate();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHwChatInput> ChatInputWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UHwChatInput> ChatInputWidgetInstance;	
	
	FString ChatMessageString;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> TimerTextWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> TimerTextWidgetInstance;
	
	

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;


};
