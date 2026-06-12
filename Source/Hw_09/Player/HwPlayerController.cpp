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
	
	//ChatInputWidgetClass가 유효한 위젯 클래스인지 확인 
	if (IsValid(ChatInputWidgetClass) == true)
	{
		// ChatInputWidgetClass를 기반으로 실제 위젯 객체를 생성
		// this 는 위쳇의 소유자 Owner역할을 한다.
		ChatInputWidgetInstance = CreateWidget<UHwChatInput>(this, ChatInputWidgetClass);
		
		// ChatinputWidgetInstance 가 있는지 확인
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			// 생성된 위젯을 화면(Viewport)에 추가한다.
			ChatInputWidgetInstance->AddToViewport();
		}
	}
	if (IsValid(NotificationTextWidgetClass) == true)
	{
		// UI 등록
		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			// 맞으면 출력
			NotificationTextWidgetInstance->AddToViewport();
		}
	}
	if (IsValid(TimerTextWidgetClass) == true)
	{
		// UI 등록
		TimerTextWidgetInstance = CreateWidget<UUserWidget>(this, TimerTextWidgetClass);
		if (IsValid(TimerTextWidgetInstance) == true)
		{
			// 맞으면 출력
			TimerTextWidgetInstance->AddToViewport();
			UE_LOG(LogTemp,Warning,TEXT("TimerTextWidgetInstance::AddToViewport()"));
		}
	}

}

void AHwPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;
	//PrintChatMessageString(InChatMessageString);
	//로컬 플레이어가 소유한 컨트롤러인지 확인합니다.
	// 이 코드를 실행하는 주체가 해당 컴퓨터가 맞는지 확인 하는 코드 이다.
	if (IsLocalController() == true)
	{
		
		AHwPlayerState* CXPS = GetPlayerState<AHwPlayerState>();
		if (IsValid(CXPS) == true)
		{
			//FString CombinedMessageString = CXPS->PlayerNameString + TEXT(": ") + InChatMessageString;
			FString CombinedMessageString = CXPS->GetPlayerInfoString() + TEXT(": ") + InChatMessageString;

			ServerRPCPrintChatMessageString(CombinedMessageString);
		}
	}
}

void AHwPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//언리얼의 디버그 출력 함수인 PrintString()을 이용해서 화면과 로그에 문자열을 출력합니다.
	// 5초간 출력합니다.
	// Player 1과 Player2 this가 다르기 때문에 
	//UKismetSystemLibrary::PrintString(this, ChatMessageString, true, true, FLinearColor::Red, 5.0f);
	
	//FString NetModeString = ChatXFunctionLibrary::GetNetModeString(this);
	//FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	//ChatXFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);
	
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
			
			//블루프린트 위젯 편집기 계층 구조(Hierarchy)에 있는 텍스트 상자의 '이름'을 정확히 적어줍니다.
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
	UE_LOG(LogTemp,Warning,TEXT("Player NotificationText::NotificationText()"));
}

void AHwPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);

}

void AHwPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	// for (TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
	// {
	// 	ACXPlayerController* CXPlayerController = *It;
	// 	if (IsValid(CXPlayerController) == true)
	// 	{
	// 		CXPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
	// 	}
	// }

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
