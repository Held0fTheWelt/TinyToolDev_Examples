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
#include "InternalIndexServiceModule.h"
#include "IISServiceInterface.h"
#include "IISAgentAccessService.h"
#include "IISAgentAccessTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISContractExposesAgentToolsTest,
	"InternalIndexService.Contract.ExposesAgentTools",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISContractExposesAgentToolsTest::RunTest(const FString& Parameters)
{
	if (!FInternalIndexServiceModule::IsAvailable())
	{
		AddWarning(TEXT("IIS module unavailable; skipping contract delegation test."));
		return true;
	}

	IInternalIndexService& Service = FInternalIndexServiceModule::Get().GetService();

	FIISAgentToolRequest Request;
	Request.ToolKind = EIISAgentToolKind::FindUsages;
	Request.SymbolName = TEXT("__iis_nonexistent_symbol__");
	Request.MaxResults = 5;

	FIISAgentToolResponse ViaContract;
	const bool bContract = Service.FindUsages(Request, ViaContract);

	FIISAgentToolResponse ViaDirect;
	const bool bDirect = FIISAgentAccessService::FindUsages(Request, ViaDirect);

	TestEqual(TEXT("Contract FindUsages success matches direct call"), bContract, bDirect);
	TestEqual(TEXT("Contract FindUsages status matches direct call"),
		static_cast<int32>(ViaContract.Status), static_cast<int32>(ViaDirect.Status));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
