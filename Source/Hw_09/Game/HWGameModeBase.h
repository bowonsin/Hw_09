// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HWGameModeBase.generated.h"

/**
 * 
 */
class AHwPlayerController;
UCLASS()
class HW_09_API AHWGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void OnPostLogin(AController* NewPlayer) override;	
	
	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);
	
	virtual void BeginPlay() override;
	
	void PrintChatMessageString(AHwPlayerController* InChattingPlayerController, const FString& InChatMessageString);
	
	void IncreaseGuessCount(AHwPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(AHwPlayerController* InChattingPlayerController, int InStrikeCount);
	

protected:
	// 1초마다 반복 호출될 타이머용 함수
	void UpdateGameTimer();
	
	FString SecretNumberString;

	TArray<TObjectPtr<AHwPlayerController>> AllPlayerControllers;
	
	// 타이머 제어용 핸들
	FTimerHandle GameTimerHandle;
	
protected:
	// 현재 플레이 중인 플레이어의 AllPlayerControllers 배열 인덱스
	int32 CurrentTurnIndex = 0;

	// 현재 턴의 플레이어가 숫자를 입력(활동)했는지 여부
	bool bHasActedInCurrentTurn = false;

	// 다음 플레이어에게 턴을 넘기는 함수
	void TurnToNextPlayer();
};
