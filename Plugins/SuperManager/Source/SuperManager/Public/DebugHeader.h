#pragma once
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "FrameWork/Notifications/NotificationManager.h"

namespace DebugHeader {
	//屏幕输出信息
	static void Print(const FString& Message, const FColor& Color) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 8.f, Color, Message);
		}
	}

	//调试窗口
	static void PrintLog(const FString& Message) {
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}

	//提示窗口
	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType, const FString& Message, bool bShowAsWarning = true) {
		if (bShowAsWarning) {
			FText MsgTitle = FText::FromString(TEXT("Warning"));

			return FMessageDialog::Open(MsgType, FText::FromString(Message), MsgTitle);
		}
		else  return FMessageDialog::Open(MsgType, FText::FromString(Message));
	}

	//编辑器调试信息（非屏幕输出）
	static void ShowNotifyInfo(const FString& Message) {
		FNotificationInfo NotifyInfo(FText::FromString(Message));
		NotifyInfo.bUseLargeFont = true;
		NotifyInfo.FadeOutDuration = 7.f;

		FSlateNotificationManager::Get().AddNotification(NotifyInfo);
	}
}



