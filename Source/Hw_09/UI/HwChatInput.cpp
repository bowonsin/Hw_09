// Fill out your copyright notice in the Description page of Project Settings.


#include "HwChatInput.h"

#include "Components/EditableTextBox.h"
#include "Hw_09/Player/HwPlayerController.h"

void UHwChatInput::NativeConstruct()
{
	Super::NativeConstruct();
	//UEditableTextBox의 이벤트에 함수를 등록하는 부분입니다. 
	//텍스트 입력 완료->OnTextCommitted 발생->OnChatInputTextCommitted 호출
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);		
	}	
}
void UHwChatInput::NativeDestruct()
{
	Super::NativeDestruct();
	
	//UEditableTextBox의 이벤트에 함수를 등록하는 부분입니다. 
	//텍스트 입력 완료->OnTextCommitted 발생->OnChatInputTextCommitted 호출

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UHwChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	//텍스트 입력이 Enter 키로 완료되었는지 확인
	if (CommitMethod == ETextCommit::OnEnter)
	{
		//현재 이 위젯을 소유하고 있는 플레이어 컨트롤러를 가져온다.
		APlayerController* OwningPlayerController = GetOwningPlayer();
		//플레이어 컨트롤러가 정상적으로 존재하는지 확인합니다.
		if (IsValid(OwningPlayerController) == true)
		{
			//기본 APlayerController를 프로젝트에서 만든 커스텀 컨트롤러 AHwPlayerController로 캐스팅합니다.
			//그래야 AHwPlayerController에만 있는 함수를 호출할 수 있습니다.
			AHwPlayerController* OwningCXPlayerController = Cast<AHwPlayerController>(OwningPlayerController);
			if (IsValid(OwningCXPlayerController) == true)
			{
				//사용자가 입력한 텍스트를 문자열(FString)로 변환하여 전달합니다.
				OwningCXPlayerController->SetChatMessageString(Text.ToString());

				EditableTextBox_ChatInput->SetText(FText());
			}
		}
	}
}
