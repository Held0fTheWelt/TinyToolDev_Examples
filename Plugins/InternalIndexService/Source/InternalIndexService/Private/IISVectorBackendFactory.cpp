/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISVectorIndexBackend.h"

#include "IISJsonlBruteForceBackend.h"
#include "IISSettings.h"

FString IISResolveConfiguredVectorBackendId()
{
	const UIISSettings* Settings = GetDefault<UIISSettings>();
	return Settings ? Settings->VectorBackend.ToLower() : TEXT("jsonl_bruteforce");
}

TUniquePtr<IIISVectorIndexBackend> CreateVectorIndexBackend()
{
	const FString BackendId = IISResolveConfiguredVectorBackendId();
	if (BackendId == TEXT("hnsw"))
	{
		extern TUniquePtr<IIISVectorIndexBackend> CreateHnswVectorIndexBackend();
		return CreateHnswVectorIndexBackend();
	}
	return MakeUnique<FIISJsonlBruteForceBackend>();
}
