// Fill out your copyright notice in the Description page of Project Settings.


#include "HwPlayerController.h"
#include "Hw_09/UI/HwChatInput.h"
#include "Hw_09/Hw_09.h"
#include "Kismet/GameplayStatics.h"
#include "Hw_09/Game/HWGameModeBase.h"
#include "Hw_09/Player/HwPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Components/TextBlock.h"

#include "EngineUtils.h"
#include "Hw_09/Game/HwGameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"


AHwPlayerController::AHwPlayerController()
{
	bReplicates = true;

}

void AHwPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	FInputModeUIOnly InputModeUIOnly;
	if (IsLocalController() == false)
	{
		return;
	}
	
	SetInputMode(InputModeUIOnly);
	
	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UHwChatInput>(this, ChatInputWidgetClass);
		
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}
	}
	if (IsValid(NotificationTextWidgetClass) == true)
	{
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			// 맞으면 출력
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
	if (IsValid(TimerTextWidgetClass) == true)
	{
		TimerTextWidgetInstance = CreateWidget<UUserWidget>(this, TimerTextWidgetClass);
		if (IsValid(TimerTextWidgetInstance) == true)
		{
			TimerTextWidgetInstance->AddToViewport();
		}
	}

}

void AHwPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;
	if (IsLocalController() == true)
	{
		
		AHwPlayerState* CXPS = GetPlayerState<AHwPlayerState>();
		if (IsValid(CXPS) == true)
		{
			FString CombinedMessageString = CXPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void AHwPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{

	if (IsLocalController() == false)
		return;
	ChatXFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);

}

void AHwPlayerController::ClientRPCTimerUiUpdate_Implementation()
{
	// 이게 문제내..
	if (IsLocalController() == false)
		return ;
	
	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (IsValid(GS) == true)
	{
		AHwGameStateBase* CXGM = Cast<AHwGameStateBase>(GS);
		if (IsValid(CXGM) == true)
		{
			if (UTextBlock* FoundTextBlock = Cast<UTextBlock>(TimerTextWidgetInstance->GetWidgetFromName(TEXT("Timer"))))
			{
				FString NewString = FString::Printf(TEXT("Time : %d"), CXGM->RemainingTime);
				FoundTextBlock->SetText(FText::FromString(NewString));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("위젯 안에서 'Timer'라는 이름의 텍스트 블록을 찾을 수 없습니다!"));
			}
		}
	}
}

void AHwPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, NotificationText);
}

void AHwPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);

}

void AHwPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		AHWGameModeBase* CXGM = Cast<AHWGameModeBase>(GM);
		if (IsValid(CXGM) == true)
		{
			CXGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}
