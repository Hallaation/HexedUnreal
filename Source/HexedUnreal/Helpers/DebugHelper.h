#pragma once

namespace DebugHelper
{
	static void Log(const FString& Message)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, Message);
	}

	static void Log(const float FloatMessage)
	{
		const FString OutMessage = FString::Printf(TEXT("%f"), FloatMessage);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, OutMessage);
	}

	static void Log(const double DoubleMessage)
	{
		const FString OutMessage = FString::Printf(TEXT("%f"), DoubleMessage);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, OutMessage);
	}
	
	static void Log(const int IntMessage)
	{
		const FString OutMessage = FString::Printf(TEXT("%d"), IntMessage);
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, OutMessage);
	}
	
}