/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IISMcpServerManager.h"

/** One registered embedding-route executor (e.g. the LLM Store bridge). */
struct FIISIntegrationStatus
{
	FString ExecutorId;
	bool bRegistered = false;
};

/** Immutable snapshot of everything the dashboard cards render. */
struct FIISPanelStatusSnapshot
{
	bool bServiceAvailable = false;
	FString ServiceVersion;
	FString IndexRoot;
	bool bCatalogPresent = false;
	bool bVectorsPresent = false;
	int32 ChunkCount = -1;
	FIISMcpStatus Mcp;
	TArray<FIISIntegrationStatus> Integrations;
};

/** Gather the current panel status. Pure read; no side effects beyond folder ensure. */
INTERNALINDEXSERVICEEDITOR_API FIISPanelStatusSnapshot IISCapturePanelStatus();
