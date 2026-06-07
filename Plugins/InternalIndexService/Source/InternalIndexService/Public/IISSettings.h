/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "IISSettings.generated.h"

/**
 * Project-level settings for the Internal Index Service.
 * Single source of truth for vector backend and MCP endpoint configuration.
 * Appears under Project Settings -> Plugins -> Internal Index Service.
 */
UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Internal Index Service"))
class INTERNALINDEXSERVICE_API UIISSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UIISSettings();

	/** Vector index backend id: "jsonl_bruteforce" (default) or "hnsw". */
	UPROPERTY(config, EditAnywhere, Category = "Index", meta = (DisplayName = "Vector Backend"))
	FString VectorBackend;

	/** Enable the local loopback (127.0.0.1) MCP endpoint for AI agents. Off by default. */
	UPROPERTY(config, EditAnywhere, Category = "MCP", meta = (DisplayName = "Enable MCP Endpoint"))
	bool bEnableMcpEndpoint;

	/** TCP port for the loopback MCP endpoint. */
	UPROPERTY(config, EditAnywhere, Category = "MCP", meta = (DisplayName = "MCP Port", ClampMin = "1", ClampMax = "65535"))
	int32 McpPort;

	/** Optional override for the on-disk index root. Empty = Saved/InternalIndexService. */
	UPROPERTY(config, EditAnywhere, Category = "Index", meta = (DisplayName = "Index Root (optional)"))
	FString IndexRoot;

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};
