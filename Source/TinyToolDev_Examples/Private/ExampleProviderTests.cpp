// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "CoreMinimal.h"
#include "ExampleCustomSimilarityProvider.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExampleProviderGroupsByPrefixTest,
	"SmartContentDietExamples.CustomProvider.GroupsByPrefix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExampleProviderGroupsByPrefixTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	FExampleCustomSimilarityProvider Provider;
	TestEqual(TEXT("Provider id is the example id."), Provider.GetProviderId(), FString(TEXT("example-prefix")));

	FSmartSimilarityRequest Request;
	Request.Mode = ESmartSimilarityMode::Perceptual;

	FSmartAssetDescriptor TreeA;
	TreeA.AssetPath = TEXT("/Game/Trees/SM_Tree_01");
	Request.Assets.Add(TreeA);

	FSmartAssetDescriptor TreeB;
	TreeB.AssetPath = TEXT("/Game/Trees/SM_Tree_02");
	Request.Assets.Add(TreeB);

	FSmartAssetDescriptor Rock;
	Rock.AssetPath = TEXT("/Game/Rocks/SM_Rock_01");
	Request.Assets.Add(Rock);

	FSmartSimilarityResult Result;
	TestTrue(TEXT("Compute succeeds."), Provider.ComputeSimilarityGroups(Request, Result));
	TestEqual(TEXT("One group (the two trees)."), Result.Groups.Num(), 1);
	TestEqual(TEXT("Group has both trees."), Result.Groups[0].Members.Num(), 2);
	return true;
}

#endif
