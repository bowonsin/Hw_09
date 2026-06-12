// Fill out your copyright notice in the Description page of Project Settings.


#include "HwGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Hw_09/Player/HwPlayerController.h"
AHwGameStateBase::AHwGameStateBase()
{
	RemainingTime = 15; 
}

void AHwGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		// 본인 클라이언트 플레이어 확인
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			// 상대 클라이언트 플레이어 확인
			AHwPlayerController* CXPC = Cast<AHwPlayerController>(PC);
			if (IsValid(CXPC) == true)
			{
				// 어떤 플레이어가 입장했는지 확인
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				CXPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
void AHwGameStateBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// RemainingTime 변수를 네트워크 동기화 대상 등록
	DOREPLIFETIME(AHwGameStateBase, RemainingTime);
}
