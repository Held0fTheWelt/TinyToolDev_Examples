# IIS MCP Proxy

Stdio MCP proxy that exposes the Internal Index Service agent tools to MCP
clients. It forwards each tool call to the loopback HTTP endpoint hosted by the
running Unreal Editor (`InternalIndexServiceEditor`). **The editor must be open
with the endpoint enabled** (`[InternalIndexService] bEnableMcpEndpoint=True` in
`Config/DefaultEngine.ini`).

## Setup

```bash
cd Plugins/InternalIndexService/Tools/mcp_proxy
python -m venv .venv && . .venv/bin/activate   # or your environment
pip install -r requirements.txt
```

## Client configuration (e.g. Claude Code)

```json
{
  "mcpServers": {
    "internal-index-service": {
      "command": "python",
      "args": [
        "-m", "iis_mcp_proxy.server",
        "--handshake", "<ProjectDir>/Saved/InternalIndexService/agent_contracts/mcp_endpoint.json"
      ],
      "cwd": "<ProjectDir>/Plugins/InternalIndexService/Tools/mcp_proxy"
    }
  }
}
```

The handshake file (host, port, session token) is written by the editor when the
endpoint starts. Tools exposed: `iis_search`, `iis_get_context_pack`,
`iis_get_chunk`, `iis_get_source_references`, `iis_find_usages`,
`iis_explain_blueprint`. All are retrieval-only — no project mutation.

## Tests

```bash
python -m pytest tests/ -v
```
