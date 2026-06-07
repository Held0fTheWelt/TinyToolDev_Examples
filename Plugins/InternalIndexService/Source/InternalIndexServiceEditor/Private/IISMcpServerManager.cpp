/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISMcpServerManager.h"

void FIISMcpServerManager::Start()
{
	if (bRunning)
	{
		return;
	}

	Endpoint = MakeUnique<FIISMcpEndpoint>();
	Endpoint->InitializeSessionToken();
	bRunning = Endpoint->Start(FIISMcpEndpoint::GetConfiguredPort());
	if (!bRunning)
	{
		Endpoint.Reset();
	}
}

void FIISMcpServerManager::Stop()
{
	if (!bRunning && !Endpoint.IsValid())
	{
		return;
	}

	if (Endpoint.IsValid())
	{
		Endpoint->Stop();
		Endpoint.Reset();
	}
	bRunning = false;
}

void FIISMcpServerManager::Restart()
{
	Stop();
	Start();
}

void FIISMcpServerManager::RotateToken()
{
	if (Endpoint.IsValid())
	{
		Endpoint->RotateToken();
	}
}

FIISMcpStatus FIISMcpServerManager::GetStatus() const
{
	FIISMcpStatus Status;
	Status.RequestCount = FIISMcpEndpoint::GetRequestLogCount();
	if (Endpoint.IsValid())
	{
		Status.bRunning = Endpoint->IsListening();
		Status.bTokenPresent = !Endpoint->GetSessionToken().IsEmpty();
		Status.Port = Endpoint->GetBoundPort() > 0 ? Endpoint->GetBoundPort() : FIISMcpEndpoint::GetConfiguredPort();
	}
	return Status;
}
