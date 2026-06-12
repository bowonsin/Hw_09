// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HwChatInput.generated.h"


class UEditableTextBox;
UCLASS()
class HW_09_API UHwChatInput : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// EditableTextBox_ChatInput 에디터블TextBox Point Object 
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;
	
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;
	
protected:
	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

};
