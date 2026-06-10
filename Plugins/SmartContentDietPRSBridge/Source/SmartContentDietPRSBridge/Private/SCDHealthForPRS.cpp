/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet PRS Bridge" Unreal Engine plugin.
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

#include "SCDHealthForPRS.h"

#include "Engine/Engine.h"
#include "ISmartContentHealthSource.h"
#include "UObject/UObjectGlobals.h"

namespace
{
#if WITH_DEV_AUTOMATION_TESTS
	TFunction<bool(const FString&, ESmartHealthSeverity&)> GQueryAssetSeverityForTests;
#endif

	const ISmartContentHealthSource* ResolveSCDHealthSource()
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

		return Cast<ISmartContentHealthSource>(GEngine->GetEngineSubsystemBase(SubsystemClass));
	}
}

bool FSCDHealthForPRS::QueryAssetSeverity(const FString& AssetPath, ESmartHealthSeverity& OutSeverity)
{
#if WITH_DEV_AUTOMATION_TESTS
	if (GQueryAssetSeverityForTests)
	{
		return GQueryAssetSeverityForTests(AssetPath, OutSeverity);
	}
#endif

	const ISmartContentHealthSource* Source = ResolveSCDHealthSource();
	if (!Source || !Source->HasSnapshot())
	{
		return false;
	}

	FSmartHealthRecord Record;
	if (!Source->QueryAssetHealth(AssetPath, Record))
	{
		return false;
	}

	OutSeverity = Record.Severity;
	return true;
}

#if WITH_DEV_AUTOMATION_TESTS
void FSCDHealthForPRS::SetQueryAssetSeverityForTests(
	TFunction<bool(const FString&, ESmartHealthSeverity&)> Resolver)
{
	GQueryAssetSeverityForTests = MoveTemp(Resolver);
}

void FSCDHealthForPRS::ResetQueryAssetSeverityForTests()
{
	GQueryAssetSeverityForTests.Reset();
}
#endif
