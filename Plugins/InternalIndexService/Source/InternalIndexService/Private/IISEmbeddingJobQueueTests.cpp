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
#include "IISEmbeddingJobQueue.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISComputeSHA256EmptyTest,
	"InternalIndexService.EmbeddingJobQueue.ComputeSHA256.Empty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISComputeSHA256EmptyTest::RunTest(const FString& Parameters)
{
	const FString Hash = FIISEmbeddingJobQueue::ComputeSHA256(TEXT(""));
	TestEqual(TEXT("SHA-256 of empty string"),
		Hash,
		FString(TEXT("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIISComputeSHA256AbcTest,
	"InternalIndexService.EmbeddingJobQueue.ComputeSHA256.Abc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIISComputeSHA256AbcTest::RunTest(const FString& Parameters)
{
	const FString Hash = FIISEmbeddingJobQueue::ComputeSHA256(TEXT("abc"));
	TestEqual(TEXT("SHA-256 of 'abc'"),
		Hash,
		FString(TEXT("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
