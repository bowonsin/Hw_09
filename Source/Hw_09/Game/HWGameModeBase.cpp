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
	CurrentTurnIndex = 0;
	bHasActedInCurrentTurn = false;
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &AHWGameModeBase::UpdateGameTimer, 1.0f, true);

}

void AHWGameModeBase::PrintChatMessageString(AHwPlayerController* InChattingPlayerController,
	const FString& InChatMessageString)
{
	if (AllPlayerControllers.IsValidIndex(CurrentTurnIndex) == false)
	{
		return;
	}
	int Index = InChatMessageString.Len() - 3;
	FString GuessNumberString = InChatMessageString.RightChop(Index);
	bool bIsGuessNumber = IsGuessNumberString(GuessNumberString);
	if (InChattingPlayerController != AllPlayerControllers[CurrentTurnIndex])
	{
		if (bIsGuessNumber)
		{
			InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("아직 당신의 턴이 아닙니다!"));
			return; 
		}
		for (const auto& PC : AllPlayerControllers)
		{
			if (IsValid(PC))
			{
				PC->ClientRPCPrintChatMessageString(InChatMessageString);
			}
		}
		return; 
	}

	if (bIsGuessNumber)
	{
		bHasActedInCurrentTurn = true;
		FString JudgeResultString = JudgeResult(SecretNumberString, GuessNumberString);
		IncreaseGuessCount(InChattingPlayerController);
		FString CombinedMessageString = InChatMessageString + TEXT(" -> ") + JudgeResultString;
		for (const auto& PC : AllPlayerControllers)
		{
			if (IsValid(PC))
			{
				PC->ClientRPCPrintChatMessageString(CombinedMessageString);
			}
		}
		// 승리 판정
		if (GuessNumberString == SecretNumberString)
		{
			JudgeGame(InChattingPlayerController, 3);
			return;
		}

		JudgeGame(InChattingPlayerController, 0);

		TurnToNextPlayer();
	}
	else
	{
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
		CXPS->CurrentGuessCount++;
	}
}

void AHWGameModeBase::ResetGame()
{
	CurrentTurnIndex = 0;
	bHasActedInCurrentTurn = false;
	SecretNumberString = GenerateSecretNumber();

	for (const auto& CXPlayerController : AllPlayerControllers)
	{
		AHwPlayerState* CXPS = CXPlayerController->GetPlayerState<AHwPlayerState>();
		if (IsValid(CXPS) == true)
		{
			
			CXPS->CurrentGuessCount = 0;

			for (const auto& PC : AllPlayerControllers)
			{
				if (IsValid(PC)) 
				{
					PC->ClientRPCPrintChatMessageString(FString::Printf(TEXT("It's Player %d's turn!"), CXPS->CurrentGuessCount + 1));
				}
			}
			
			
		}
	}
}
void AHWGameModeBase::JudgeGame(AHwPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (InStrikeCount == 3)
	{
		AHwPlayerState* WinnerPS =
			InChattingPlayerController->GetPlayerState<AHwPlayerState>();

		if (IsValid(WinnerPS))
		{
			FString Msg =
				WinnerPS->PlayerNameString +
				TEXT(" has won the game.");

			for (const auto& PC : AllPlayerControllers)
			{
				if (IsValid(PC))
				{
					PC->NotificationText =
						FText::FromString(Msg);
				}
			}
		}

		ResetGame();
		return;
	}

	bool bIsDraw = true;

	for (const auto& PC : AllPlayerControllers)
	{
		AHwPlayerState* PS =
			PC->GetPlayerState<AHwPlayerState>();

		if (IsValid(PS))
		{
			if (PS->CurrentGuessCount < PS->MaxGuessCount)
			{
				bIsDraw = false;
				break;
			}
		}
	}

	if (bIsDraw)
	{
		for (const auto& PC : AllPlayerControllers)
		{
			if (IsValid(PC))
			{
				PC->NotificationText =
					FText::FromString(TEXT("Draw"));
			}
		}

		ResetGame();
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
					FString TimeOutMsg = TEXT(" 시간 초과로 인해 기회가 1회 차감됩니다!");
					CurrentPC->ClientRPCPrintChatMessageString(TimeOutMsg);
                
					JudgeGame(CurrentPC, 0);
					TurnToNextPlayer();
				}
				GS->ResetTime();
			}
		}
		else
		{
			GS->RemainingTime--;
			
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
			FString TurnMsg = FString::Printf(TEXT("It's %s's turn!"), *PS->PlayerNameString);
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

