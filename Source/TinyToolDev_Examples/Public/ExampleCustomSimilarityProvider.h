// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISmartSimilarityProvider.h"

/** Teaching example: groups assets that share a folder prefix. Replace with real logic. */
class FExampleCustomSimilarityProvider : public ISmartSimilarityProvider
{
public:
	virtual FString GetProviderId() const override { return TEXT("example-prefix"); }
	virtual bool SupportsMode(ESmartSimilarityMode Mode) const override
	{
		return Mode == ESmartSimilarityMode::Perceptual;
	}
	virtual bool SupportsAssetClass(FName) const override { return true; }
	virtual bool ComputeSimilarityGroups(
		const FSmartSimilarityRequest& Request,
		FSmartSimilarityResult& OutResult) override;
};
