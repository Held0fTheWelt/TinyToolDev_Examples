/* Copyright (c) 2025-2026 Yves Tanas
 * License-Identifier: LicenseRef-Fab-Standard-EULA */

#include "IISSettings.h"

UIISSettings::UIISSettings()
	: VectorBackend(TEXT("jsonl_bruteforce"))
	, bEnableMcpEndpoint(false)
	, McpPort(8731)
	, IndexRoot()
{
}
