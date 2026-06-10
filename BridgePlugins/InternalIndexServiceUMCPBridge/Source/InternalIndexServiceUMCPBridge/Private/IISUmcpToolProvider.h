/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IUmcpToolProvider.h"

/**
 * Publishes the six IIS agent tools to the Unified MCP Server. Descriptors are
 * sourced from IIS's own authoritative tool manifest (DRY); invocation maps the
 * local tool name to EIISAgentToolKind and calls IIS's agent executor.
 */
class FIISUmcpToolProvider : public IUmcpToolProvider
{
public:
	virtual FString GetProviderId() const override { return TEXT("iis"); }
	virtual FString GetProviderDisplayName() const override { return TEXT("Internal Index Service"); }
	virtual void GetToolDescriptors(TArray<FUmcpToolDescriptor>& OutTools) const override;
	virtual void InvokeTool(const FUmcpToolInvocation& Invocation, FUmcpToolResult& OutResult) override;
	virtual bool IsReady() const override;
};
