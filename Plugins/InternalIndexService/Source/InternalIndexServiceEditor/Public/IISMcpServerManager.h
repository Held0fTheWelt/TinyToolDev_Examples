/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#pragma once

#include "CoreMinimal.h"
#include "IISMcpEndpoint.h"
#include "Templates/UniquePtr.h"

struct FIISMcpStatus
{
	bool bRunning = false;
	int32 Port = 0;
	bool bTokenPresent = false;
	int64 RequestCount = 0;
};

class INTERNALINDEXSERVICEEDITOR_API FIISMcpServerManager
{
public:
	void Start();
	void Stop();
	void Restart();

	bool IsRunning() const { return bRunning; }

	void RotateToken();

	FIISMcpStatus GetStatus() const;

private:
	TUniquePtr<FIISMcpEndpoint> Endpoint;
	bool bRunning = false;
};
