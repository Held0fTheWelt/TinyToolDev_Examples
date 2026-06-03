// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#include "ExampleCustomSimilarityProvider.h"

#include "Misc/Paths.h"

bool FExampleCustomSimilarityProvider::ComputeSimilarityGroups(
	const FSmartSimilarityRequest& Request,
	FSmartSimilarityResult& OutResult)
{
	TMap<FString, TArray<FString>> ByFolder;
	for (const FSmartAssetDescriptor& Descriptor : Request.Assets)
	{
		ByFolder.FindOrAdd(FPaths::GetPath(Descriptor.AssetPath)).Add(Descriptor.AssetPath);
	}

	for (const TPair<FString, TArray<FString>>& Pair : ByFolder)
	{
		if (Pair.Value.Num() < 2)
		{
			continue;
		}

		FSmartSimilarityGroup Group;
		Group.Members = Pair.Value;
		Group.Confidence = 0.6f;
		Group.bApproximate = true;
		Group.Signals.Add(ESmartSimilaritySignal::Perceptual);
		Group.Rationale = TEXT("Example: same folder prefix.");
		OutResult.Groups.Add(Group);
	}

	return true;
}
