// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "TinyToolDev_ExamplesModule.h"

#include "Engine/Engine.h"
#include "ExampleCustomSimilarityProvider.h"
#include "ISmartContentDietRegistry.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FTinyToolDevExamplesModule, TinyToolDev_Examples, "TinyToolDev_Examples");

namespace
{
	ISmartContentDietRegistry* ResolveSCDRegistry()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		UClass* SubsystemClass =
			FindObject<UClass>(nullptr, TEXT("/Script/SmartContentDiet.SmartContentDietSubsystem"));
		if (!SubsystemClass)
		{
			SubsystemClass =
				LoadObject<UClass>(nullptr, TEXT("/Script/SmartContentDiet.SmartContentDietSubsystem"));
		}
		if (!SubsystemClass)
		{
			return nullptr;
		}

		return Cast<ISmartContentDietRegistry>(GEngine->GetEngineSubsystemBase(SubsystemClass));
	}
}

void FTinyToolDevExamplesModule::StartupModule()
{
	if (ISmartContentDietRegistry* Registry = ResolveSCDRegistry())
	{
		ExampleProvider = MakeShared<FExampleCustomSimilarityProvider>();
		Registry->RegisterSimilarityProvider(ExampleProvider.ToSharedRef(), ESmartProviderTier::ThirdParty);
	}
}

void FTinyToolDevExamplesModule::ShutdownModule()
{
	if (ExampleProvider.IsValid())
	{
		if (ISmartContentDietRegistry* Registry = ResolveSCDRegistry())
		{
			Registry->UnregisterSimilarityProvider(ExampleProvider.ToSharedRef());
		}
		ExampleProvider.Reset();
	}
}
