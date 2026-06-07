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

#pragma once

#include "CoreMinimal.h"

class INTERNALINDEXSERVICE_API FIISStoragePaths
{
public:
	static FString GetDefaultIndexRoot();
	static FString GetImportsDir();
	static FString GetIndexesDir();
	static FString GetChunksDir();
	static FString GetChunkImportsDir();
	static FString GetSymbolsDir();
	static FString GetContextPacksDir();
	static FString GetLogsDir();
	static FString GetReportsDir();
	static FString GetEmbeddingsDir();
	static FString GetVectorsDir();
	static FString GetAgentDir();
	static FString GetAgentContractsDir();
	static FString GetAgentRunsDir();

	static TArray<FString> GetRequiredDirectoryPaths();
	static bool EnsureDefaultFolders(TArray<FString>* OutCreatedFolders = nullptr, TArray<FString>* OutWarnings = nullptr);
};
