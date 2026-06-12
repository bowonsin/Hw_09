// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "HwGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class HW_09_API AHwGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	
	AHwGameStateBase();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(const FString& InNameString = FString(TEXT("XXXXXXX")));
	
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// [시간 체크] 서버에서 동기화되어 클라이언트 UI가 참조할 남은 시간
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameMode|Timer")
	int32 RemainingTime;
	
	void ResetTime() { RemainingTime = TurnTime; }
	
	const int32 TurnTime = 15;
};
