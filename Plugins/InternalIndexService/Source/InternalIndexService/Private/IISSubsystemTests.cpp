/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "Internal Index Service" Unreal Engine plugin.
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

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "IISSubsystem.h"
#include "InternalIndexServiceModule.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISSubsystemDelegatesToModuleTest,
	"InternalIndexService.Subsystem.DelegatesToModule",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISSubsystemDelegatesToModuleTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is available"), GEngine);
	if (!GEngine)
	{
		return false;
	}

	UIISSubsystem* Subsystem = GEngine->GetEngineSubsystem<UIISSubsystem>();
	TestNotNull(TEXT("UIISSubsystem resolves"), Subsystem);
	if (!Subsystem)
	{
		return false;
	}

	if (!FInternalIndexServiceModule::IsAvailable())
	{
		AddWarning(TEXT("IIS module unavailable; only resolution checked."));
		return true;
	}

	IInternalIndexService& Service = FInternalIndexServiceModule::Get().GetService();

	TestEqual(TEXT("Version matches module getter"),
		Subsystem->GetServiceVersion(), Service.GetServiceVersion());
	TestEqual(TEXT("IndexRoot matches module getter"),
		Subsystem->GetDefaultIndexRoot(), Service.GetDefaultIndexRoot());
	TestEqual(TEXT("Availability matches module getter"),
		Subsystem->IsAvailable(), Service.IsAvailable());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
