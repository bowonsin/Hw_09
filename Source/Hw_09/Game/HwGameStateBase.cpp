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
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			AHwPlayerController* CXPC = Cast<AHwPlayerController>(PC);
			if (IsValid(CXPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				CXPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
void AHwGameStateBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHwGameStateBase, RemainingTime);
}
