// Fill out your copyright notice in the Description page of Project Settings.


#include "HWGameModeBase.h"
#include "HwGameStateBase.h"
#include "Hw_09/Player/HwPlayerController.h"
#include "Hw_09/Player/HwPlayerState.h"
#include "EngineUtils.h"


void AHWGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	SecretNumberString = GenerateSecretNumber();
	UE_LOG(LogTemp,Error,TEXT("%s"),*SecretNumberString)
	
	
	// 현재 플레이 중인 플레이어의 AllPlayerControllers 배열 인덱스
	CurrentTurnIndex = 0;
	// 현재 턴의 플레이어가 숫자를 입력(활동)했는지 여부
	bHasActedInCurrentTurn = false;
	
	// 게임이 시작되면 1.0초마다 UpdateGameTimer 함수를 '반복(true)' 호출하도록 세팅
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AHWGameModeBase::UpdateGameTimer, 1.0f, true);

}

void AHWGameModeBase::PrintChatMessageString(AHwPlayerController* InChattingPlayerController,
	const FString& InChatMessageString)
{
	if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex) == false)
	{
		return;
	}
	// 입력된 메시지의 뒤에서 3번째 자리 인덱스를 계산
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	// 잘라낸 3글자가 야구 게임에 적합한 '유효한 숫자' 구조인지 검사
	bool bIsGuessNumber = IsGuessNumberString(GuessNumberString);
	// 2. 턴 검사 수행
	if (InChattingPlayerController != AllPlayerControllers[CurrentTurnIndex])
	{
		//  [턴이 아닌 경우] 숫자를 입력했다면? 아예 무시하거나 본인에게만 경고 전송
		if (bIsGuessNumber)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("[System] 아직 당신의 턴이 아닙니다!"));
			return; //
		}
        
		//  [턴이 아닌 경우] 일반 대화라면? 정상적으로 전체 채팅 전송
		for (const auto& PC : AllPlayerControllers)
			
		{
			if (IsValid(PC))
			{
				PC->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
		return; 
	}

	// ---  여기서부터는 '자기 턴이 확실한 플레이어'만 내려올 수 있습니다 ---
	if (bIsGuessNumber)
	{
		bHasActedInCurrentTurn = true;
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		// 해당 플레이어의 시도 횟수를 1 증가
		IncreaseGuessCount(InChattingPlayerController);
		// 월드(게임)에 존재하는 모든 플레이어 컨트롤러를 순회하는 반복문
		for (TActorIterator<AHwPlayerController> It(GetWorld()); It; ++It)
		{
			AHwPlayerController* CXPlayerController = *It;
			if (IsValid(CXPlayerController) == true)
			{
				// 원래 채팅 내용 뒤에 판정 결과를 붙임 (예: "정답은 123 -> 1S0B")
				FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
				
				// 모든 플레이어의 화면(클라이언트)에 이 메시지를 출력하라고 명령(ClientRPC)
				CXPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
				
				// 판정 결과 문자열의 맨 앞 1글자를 숫자로 바꾸어 스트라이크 개수를 추출
				int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
				
				// 게임이 끝났는지(3스트라이크 혹은 기회 소진) 판정
				JudgeGame(InChattingPlayerController, StrikeCount);
			}
		}
		TurnToNextPlayer();
	}
	else
	{
		// 자기 턴인 사람이 숫자가 아닌 일반 대화를 한 경우
		for (const auto& PC : AllPlayerControllers)
		{
			if (IsValid(PC)) PC->ClientRPCPrintChatMessageString(InChatMessageString);
		}
	}
}

void AHWGameModeBase::IncreaseGuessCount(AHwPlayerController* InChattingPlayerController)
{
	AHwPlayerState* CXPS = InChattingPlayerController->GetPlayerState<AHwPlayerState>();
	if (IsValid(CXPS) == true)
	{
		// 플레이어 상태 창의 현재 시도 횟수 변수를 1 올림
		CXPS->CurrentGuessCount++;
	}
}

void AHWGameModeBase::ResetGame()
{
	//게임 리셋 시 턴도 처음으로
	CurrentTurnIndex = 0;
	bHasActedInCurrentTurn = false;
	SecretNumberString = GenerateSecretNumber();

	for (const auto& CXPlayerController : AllPlayerControllers)
	{
		AHwPlayerState* CXPS = CXPlayerController->GetPlayerState<AHwPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->CurrentGuessCount = 0;
		}
	}
}
void AHWGameModeBase::JudgeGame(AHwPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		AHwPlayerState* CXPS = InChattingPlayerController->GetPlayerState<AHwPlayerState>();
		for (const auto& CXPlayerController : AllPlayerControllers)
		{
			if (IsValid(CXPS) == true)
			{
				// "PlayerX가 게임에서 승리했습니다" 문구를 만들어서
				FString CombinedMessageString = CXPS->PlayerNameString + TEXT(" has won the game.");
				
				// 모든 플레이어 컨트롤러의 알림 텍스트 변수에 넣어줌
				CXPlayerController->NotificationText = FText::FromString(CombinedMessageString);

				// 한 판이 끝났으니 판을 새로 짬
				ResetGame();
			}
		}
	}
	else
	{
		bool bIsDraw = true;
		// 모든 플레이어를 조사해봄
		for (const auto& CXPlayerController : AllPlayerControllers)
		{
			AHwPlayerState* CXPS = CXPlayerController->GetPlayerState<AHwPlayerState>();
			if (IsValid(CXPS) == true)
			{
				// 단 한 명이라도 아직 최대 기회(MaxGuessCount)보다 적게 시도했다면 무승부가 아님(계속 진행)
				if (CXPS->CurrentGuessCount < CXPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}
		
		// 모든 플레이어의 기회가 전부 동나서 bIsDraw가 true로 유지되었다면 무승부 처리
		if (true == bIsDraw)
		{
			for (const auto& CXPlayerController : AllPlayerControllers)
			{
				// 모든 이에게 Draw... 알림을 전송하고 게임 초기화
				CXPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));

				ResetGame();
			}
		}
	}
}

void AHWGameModeBase::UpdateGameTimer()
{
	AHwGameStateBase* GS = GetGameState<AHwGameStateBase>();
	if (IsValid(GS))
	{
		if (GS->RemainingTime <= 0)
		{
			if (!bHasActedInCurrentTurn && AllPlayerControllers.IsValidIndex(CurrentTurnIndex))
			{
				AHwPlayerController* CurrentPC = AllPlayerControllers[CurrentTurnIndex];
				if (IsValid(CurrentPC))
				{
					IncreaseGuessCount(CurrentPC);
					FString TimeOutMsg = TEXT("[System] 시간 초과로 인해 기회가 1회 차감됩니다!");
					CurrentPC->ClientRPCPrintChatMessageString(TimeOutMsg);
                
					JudgeGame(CurrentPC, 0);
				}
				GS->ResetTime();
			}
		}
		else
		{
			// 서버에서 시간을 1초 줄임 -> 클라이언트들로 자동 복제(Replication)됨
			GS->RemainingTime--;
			UE_LOG(LogTemp, Warning, TEXT("%d"),GS->RemainingTime);
			for (TActorIterator<AHwPlayerController> It(GetWorld()); It; ++It)
			{
				AHwPlayerController* CXPlayerController = *It;
				if (IsValid(CXPlayerController) == true)
				{
					CXPlayerController->ClientRPCTimerUiUpdate();
				}
			}
		}
	}
}

void AHWGameModeBase::TurnToNextPlayer()
{
	if (AllPlayerControllers.Num() == 0) return;

	CurrentTurnIndex = (CurrentTurnIndex + 1) % AllPlayerControllers.Num();

	bHasActedInCurrentTurn = false;

	AHwGameStateBase* GS = GetGameState<AHwGameStateBase>();
	if (IsValid(GS))
	{
		GS->ResetTime();
	}
	if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex))
	{
		AHwPlayerController* NextPC = AllPlayerControllers[CurrentTurnIndex];
		AHwPlayerState* PS = NextPC ? NextPC->GetPlayerState<AHwPlayerState>() : nullptr;
		if (PS)
		{
			FString TurnMsg = FString::Printf(TEXT("[System] It's %s's turn!"), *PS->PlayerNameString);
			for (const auto& PC : AllPlayerControllers)
			{
				if (IsValid(PC)) 
				{
					PC->ClientRPCPrintChatMessageString(TurnMsg);
				}
			}
		}
	}
}

void AHWGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
		
	AHwPlayerController* CXPlayerController = Cast<AHwPlayerController>(NewPlayer);
	if (IsValid(CXPlayerController) == true)
	{
		CXPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		AllPlayerControllers.Add(CXPlayerController);
		AHwPlayerState* CXPS = CXPlayerController->GetPlayerState<AHwPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}
		AHwGameStateBase* CXGameStateBase =  GetGameState<AHwGameStateBase>();
		if (IsValid(CXGameStateBase) == true)
		{
			CXGameStateBase->MulticastRPCBroadcastLoginMessage(CXPS->PlayerNameString);
		}
	}
}

FString AHWGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}
	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });
	
	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool AHWGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	bool bCanPlay = false;

	do {
		if (InNumberString.Len() != 3)
		{
			break;
		}

		bool bIsUnique = true;
		TSet<TCHAR> UniqueDigits;
		for (TCHAR C : InNumberString)
		{
			if (FChar::IsDigit(C) == false || C == '0')
			{
				bIsUnique = false;
				break;
			}
			UniqueDigits.Add(C);
		}

		if (bIsUnique == false)
		{
			break;
		}

		bCanPlay = true;
		
	} while (false);	

	return bCanPlay;
}

FString AHWGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		// 자릿수도 같고 문자도 완벽히 일치하면 Strike
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else 
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;				
			}
		}
	}
	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

