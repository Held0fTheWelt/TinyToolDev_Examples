/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA
 *
 * This file is part of the "SmartContentDiet IIS Similarity Bridge" Unreal Engine plugin.
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

#include "CoreMinimal.h"
#include "IISSearchTypes.h"
#include "Misc/AutomationTest.h"
#include "SCDIISSimilarityGrouping.h"
#include "SmartSimilarityContracts.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSCDIISSimilarityGroupingTest,
	"SmartContentDietBridge.Similarity.Grouping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSCDIISSimilarityGroupingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	const TSet<FString> Known = { TEXT("/Game/A"), TEXT("/Game/B"), TEXT("/Game/C") };

	// Mixed response: one qualifying request peer, a self-hit, a foreign IIS
	// chunk id, and a below-threshold request asset.
	FIISSearchResponse Response;
	{
		FIISSearchResult Peer;
		Peer.ChunkId = TEXT("/Game/B");
		Peer.Score = 0.91f;
		Response.Results.Add(Peer);

		FIISSearchResult SelfHit;
		SelfHit.ChunkId = TEXT("/Game/A");
		SelfHit.Score = 0.99f;
		Response.Results.Add(SelfHit);

		FIISSearchResult Foreign;
		Foreign.ChunkId = TEXT("iis::code::1234");
		Foreign.Score = 0.95f;
		Response.Results.Add(Foreign);

		FIISSearchResult BelowThreshold;
		BelowThreshold.ChunkId = TEXT("/Game/C");
		BelowThreshold.Score = 0.40f;
		Response.Results.Add(BelowThreshold);
	}

	FSmartSimilarityGroup Group;
	const bool bFormed = SCDIISSimilarityGrouping::BuildSemanticGroup(
		TEXT("/Game/A"), Known, Response, 0.85f, Group);

	TestTrue(TEXT("Group forms with a qualifying peer."), bFormed);
	TestEqual(TEXT("Self plus exactly one peer."), Group.Members.Num(), 2);
	TestTrue(TEXT("Self is a member."), Group.Members.Contains(TEXT("/Game/A")));
	TestTrue(TEXT("Qualifying request asset is a member."), Group.Members.Contains(TEXT("/Game/B")));
	TestFalse(TEXT("Foreign IIS chunk id is not a member."), Group.Members.Contains(TEXT("iis::code::1234")));
	TestFalse(TEXT("Below-threshold asset is excluded."), Group.Members.Contains(TEXT("/Game/C")));
	TestTrue(TEXT("Semantic signal set."), Group.Signals.Contains(ESmartSimilaritySignal::Semantic));
	TestEqual(TEXT("Confidence reflects best peer score."), Group.Confidence, 0.91f);

	// No qualifying peer → no group, and the out-group is reset empty.
	FIISSearchResponse Empty;
	FSmartSimilarityGroup None;
	const bool bNone = SCDIISSimilarityGrouping::BuildSemanticGroup(
		TEXT("/Game/A"), Known, Empty, 0.85f, None);
	TestFalse(TEXT("No peers yields no group."), bNone);
	TestEqual(TEXT("Empty group has no members."), None.Members.Num(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
