/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet UII Migration Bridge" Unreal Engine plugin.
 * Use of this software is governed by the Fab Standard End User License Agreement
 * (EULA) applicable to this product, available at:
 * https://www.fab.com/eula
 *
 * Except as expressly permitted by the Fab Standard EULA, any reproduction,
 * distribution, modification, or use of this software, in whole or in part,
 * is strictly prohibited.
 *
 * This software is provided on an "AS IS" basis, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied, including but not
 * limited to warranties of merchantability, fitness for a particular purpose,
 * and non-infringement.
 * available at: https://www.fab.com/eula.  */

#include "SmartContentDietUIIMigrationBridgeModule.h"

#include "Engine/Engine.h"
#include "ISmartContentDietRegistry.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_MODULE(FSmartContentDietUIIMigrationBridgeModule, SmartContentDietUIIMigrationBridge)

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

void FSmartContentDietUIIMigrationBridgeModule::StartupModule()
{
	if (ISmartContentDietRegistry* Registry = ResolveSCDRegistry())
	{
		TSharedRef<FUIIMigrationAdvisorProvider> Candidate = MakeShared<FUIIMigrationAdvisorProvider>();
		if (Candidate->IsImplementationAvailable())
		{
			Provider = Candidate;
			Registry->RegisterMigrationProvider(Provider.ToSharedRef(), ESmartProviderTier::FirstParty);
		}
	}
}

void FSmartContentDietUIIMigrationBridgeModule::ShutdownModule()
{
	if (Provider.IsValid())
	{
		if (ISmartContentDietRegistry* Registry = ResolveSCDRegistry())
		{
			Registry->UnregisterMigrationProvider(Provider.ToSharedRef());
		}
		Provider.Reset();
	}
}
