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
#include "IISImportTypes.h"

class INTERNALINDEXSERVICE_API FIISPreparedChunkImporter
{
public:
	static bool ImportPreparedChunks(
		const FIISImportInputFiles& InputFiles,
		FString& OutImportReportPath,
		TArray<FString>& OutWarnings);

private:
	static bool ValidateInputFiles(
		const FIISImportInputFiles& InputFiles,
		FIISImportReport& Report);

	static bool LoadPreparedChunksJsonl(
		const FIISImportInputFiles& InputFiles,
		FIISImportReport& Report,
		TArray<FIISIndexChunk>& OutChunks,
		TMap<FString, FString>& OutChunkJsonById);

	static bool ConvertPreparedChunkJsonToIndexChunk(
		const TSharedPtr<class FJsonObject>& ChunkObject,
		const TSharedPtr<class FJsonObject>& PreparedManifestObject,
		const int32 SourceLineNumber,
		FIISIndexChunk& OutChunk,
		FIISChunkImportRecord& OutRecord,
		TArray<FString>& OutErrors);

	static bool LoadExistingChunkIds(
		const FString& ChunkStorePath,
		TSet<FString>& OutExistingChunkIds,
		TMap<FString, FString>& OutExistingChunkJsonById,
		TArray<FString>& OutWarnings);

	static bool AppendImportedChunksToStore(
		const TArray<FIISIndexChunk>& Chunks,
		const TMap<FString, FString>& ChunkJsonById,
		const FString& ChunkStorePath,
		const FString& ImportedChunksPath,
		FIISImportReport& Report);

	static bool WriteImportManifest(
		const FIISImportReport& Report);

	static bool WriteImportReportJson(
		const FIISImportReport& Report,
		const FString& ReportPath);

	static bool WriteImportReportMarkdown(
		const FIISImportReport& Report,
		const FString& ReportPath);

	static bool WriteChunkStoreManifest(
		const FIISImportReport& Report,
		const FString& ChunkStorePath);
};
