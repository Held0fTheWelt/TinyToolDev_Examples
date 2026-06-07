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

#include "IISStoragePaths.h"

#include "IISSettings.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

namespace
{
	FString CombineIISPath(const TCHAR* Leaf)
	{
		return FIISStoragePaths::GetDefaultIndexRoot() / Leaf;
	}
}

FString FIISStoragePaths::GetDefaultIndexRoot()
{
	const UIISSettings* Settings = GetDefault<UIISSettings>();
	if (Settings && !Settings->IndexRoot.IsEmpty())
	{
		return FPaths::ConvertRelativePathToFull(Settings->IndexRoot);
	}
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("InternalIndexService"));
}

FString FIISStoragePaths::GetImportsDir()
{
	return CombineIISPath(TEXT("imports"));
}

FString FIISStoragePaths::GetIndexesDir()
{
	return CombineIISPath(TEXT("indexes"));
}

FString FIISStoragePaths::GetChunksDir()
{
	return CombineIISPath(TEXT("chunks"));
}

FString FIISStoragePaths::GetChunkImportsDir()
{
	return GetChunksDir() / TEXT("imports");
}

FString FIISStoragePaths::GetSymbolsDir()
{
	return CombineIISPath(TEXT("symbols"));
}

FString FIISStoragePaths::GetContextPacksDir()
{
	return CombineIISPath(TEXT("context_packs"));
}

FString FIISStoragePaths::GetLogsDir()
{
	return CombineIISPath(TEXT("logs"));
}

FString FIISStoragePaths::GetReportsDir()
{
	return CombineIISPath(TEXT("reports"));
}

FString FIISStoragePaths::GetEmbeddingsDir()
{
	return CombineIISPath(TEXT("embeddings"));
}

FString FIISStoragePaths::GetVectorsDir()
{
	return CombineIISPath(TEXT("vectors"));
}

FString FIISStoragePaths::GetAgentDir()
{
	return CombineIISPath(TEXT("agent"));
}

FString FIISStoragePaths::GetAgentContractsDir()
{
	return GetAgentDir() / TEXT("contracts");
}

FString FIISStoragePaths::GetAgentRunsDir()
{
	return GetAgentDir() / TEXT("runs");
}

TArray<FString> FIISStoragePaths::GetRequiredDirectoryPaths()
{
	TArray<FString> Paths;
	Paths.Add(GetDefaultIndexRoot());
	Paths.Add(GetImportsDir());
	Paths.Add(GetIndexesDir());
	Paths.Add(GetChunksDir());
	Paths.Add(GetChunkImportsDir());
	Paths.Add(GetSymbolsDir());
	Paths.Add(GetContextPacksDir());
	Paths.Add(GetLogsDir());
	Paths.Add(GetReportsDir());
	Paths.Add(GetEmbeddingsDir());
	Paths.Add(GetVectorsDir());
	Paths.Add(GetAgentDir());
	Paths.Add(GetAgentContractsDir());
	Paths.Add(GetAgentRunsDir());
	return Paths;
}

bool FIISStoragePaths::EnsureDefaultFolders(TArray<FString>* OutCreatedFolders, TArray<FString>* OutWarnings)
{
	bool bAllCreatedOrPresent = true;
	IFileManager& FileManager = IFileManager::Get();

	for (const FString& DirectoryPath : GetRequiredDirectoryPaths())
	{
		if (FileManager.DirectoryExists(*DirectoryPath))
		{
			continue;
		}

		if (FileManager.MakeDirectory(*DirectoryPath, true))
		{
			if (OutCreatedFolders)
			{
				OutCreatedFolders->Add(DirectoryPath);
			}
			continue;
		}

		bAllCreatedOrPresent = false;
		if (OutWarnings)
		{
			OutWarnings->Add(FString::Printf(TEXT("Failed to create IIS directory: %s"), *DirectoryPath));
		}
	}

	return bAllCreatedOrPresent;
}
