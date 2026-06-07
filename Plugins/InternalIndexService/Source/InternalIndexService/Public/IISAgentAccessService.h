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
#include "IISAgentAccessTypes.h"
#include "IISUsageGraphTypes.h"

class INTERNALINDEXSERVICE_API FIISAgentAccessService
{
public:
	static bool ExecuteAgentTool(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse);

	static bool Search(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse);

	static bool GetContextPack(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse);

	static bool GetChunk(
		const FString& ChunkId,
		FIISAgentToolResponse& OutResponse);

	static bool GetSourceReferences(
		const FString& ChunkId,
		FIISAgentToolResponse& OutResponse);

	static bool FindUsages(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse);

	static bool QueryUsages(const FString& Query, FIISUsageQueryResult& OutResult);

	static bool ExplainBlueprint(
		const FIISAgentToolRequest& Request,
		FIISAgentToolResponse& OutResponse);

	static bool WriteAgentToolContracts(FString& OutContractsPath);

	static bool ExecuteAgentToolFromJson(
		const FString& InputJsonPath,
		FString& OutResponseJsonPath);

	static FString GetLatestResponsePath(EIISAgentToolKind ToolKind);
	static FString GetAgentContractsPath();

private:
	static void AddDefaultGuardrails(FIISAgentToolResponse& Response);

	static bool WriteAgentResponse(
		const FIISAgentToolResponse& Response,
		const FString& BaseFileName);

	static bool WriteAgentResponseJson(
		const FIISAgentToolResponse& Response,
		const FString& JsonPath);

	static bool WriteAgentResponseMarkdown(
		const FIISAgentToolResponse& Response,
		const FString& MarkdownPath);
};
