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
#include "HttpRouteHandle.h"

class IHttpRouter;

/**
 * Loopback HTTP endpoint exposing the IIS agent tools to external MCP clients.
 * Transport (IHttpRouter) is separated from logic: the Handle* methods are pure
 * functions over strings, unit-testable without sockets. They route every tool
 * call through the authoritative module getter / UIISSubsystem.
 */
class FIISMcpEndpoint
{
public:
	static bool IsEnabledByConfig();
	static int32 GetConfiguredPort();

	FIISMcpEndpoint();

	void InitializeSessionToken();
	const FString& GetSessionToken() const { return SessionToken; }
	void RotateToken();

	static FString GetHandshakeFilePath();
	void WriteHandshakeFile(int32 Port) const;

	static void AppendRequestLog(
		const FString& Tool,
		int32 Status,
		double LatencyMs,
		const FString& ResponsePath,
		const FString& ErrorCode);

	static int64 GetRequestLogCount();

	void HandleHealth(int32& OutStatus, FString& OutBody) const;

	void HandleToolsList(const FString& AuthToken, int32& OutStatus, FString& OutBody) const;

	void HandleToolsCall(const FString& Body, const FString& AuthToken, int32& OutStatus, FString& OutBody);

	bool Start(int32 Port);
	void Stop();

	bool IsListening() const { return bIsListening; }
	int32 GetBoundPort() const { return BoundPort; }

private:
	bool IsAuthorized(const FString& AuthToken) const;

	FString SessionToken;
	int32 BoundPort = 0;
	bool bIsListening = false;
	TSharedPtr<IHttpRouter> Router;
	TArray<FHttpRouteHandle> RouteHandles;
};
