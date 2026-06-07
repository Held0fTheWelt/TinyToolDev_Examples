"""MCP stdio server exposing IIS tools via the loopback endpoint.

Run: python -m iis_mcp_proxy.server --host 127.0.0.1 --port 8731 --token <tok>
If --token is omitted, it is read from the handshake file written by the editor.
"""

import argparse
import asyncio
import json
import os

import mcp.server.stdio
import mcp.types as types
from mcp.server import Server

from .client import EndpointClient


def load_handshake(handshake_path):
    with open(handshake_path, "r", encoding="utf-8") as handle:
        data = json.load(handle)
    return data["host"], int(data["port"]), data["token"]


def build_server(client: EndpointClient) -> Server:
    server = Server("internal-index-service")

    @server.list_tools()
    async def list_tools():
        return [
            types.Tool(
                name=tool["name"],
                description=tool["description"],
                inputSchema=tool["inputSchema"],
            )
            for tool in client.list_tools()
        ]

    @server.call_tool()
    async def call_tool(name: str, arguments: dict):
        result = client.call_tool(name, arguments)
        return [types.TextContent(type="text", text=json.dumps(result, indent=2))]

    return server


async def _run(server: Server):
    async with mcp.server.stdio.stdio_server() as (read_stream, write_stream):
        await server.run(read_stream, write_stream, server.create_initialization_options())


def main():
    parser = argparse.ArgumentParser(description="IIS MCP stdio proxy")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8731)
    parser.add_argument("--token", default=None)
    parser.add_argument("--handshake", default=os.environ.get("IIS_MCP_HANDSHAKE"))
    args = parser.parse_args()

    host, port, token = args.host, args.port, args.token
    if token is None and args.handshake:
        host, port, token = load_handshake(args.handshake)
    if token is None:
        raise SystemExit("No token provided; pass --token or --handshake.")

    client = EndpointClient(host, port, token)
    asyncio.run(_run(build_server(client)))


if __name__ == "__main__":
    main()
