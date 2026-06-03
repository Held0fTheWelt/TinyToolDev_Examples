// Copyright (c) 2026 Tiny Tool Development. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FExampleCustomSimilarityProvider;

class FTinyToolDevExamplesModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FExampleCustomSimilarityProvider> ExampleProvider;
};
