"""HTTP client for the IIS loopback MCP endpoint, with injectable transport."""

import json
import urllib.error
import urllib.request


class UrllibHttp:
    """Default transport using urllib (no third-party dependency required)."""

    def get(self, url, headers):
        req = urllib.request.Request(url, headers=headers, method="GET")
        with urllib.request.urlopen(req) as resp:
            return resp.status, resp.read().decode("utf-8")

    def post(self, url, headers, body):
        data = body.encode("utf-8")
        req = urllib.request.Request(url, data=data, headers=headers, method="POST")
        with urllib.request.urlopen(req) as resp:
            return resp.status, resp.read().decode("utf-8")


def _translate_endpoint_error(exc):
    message = str(exc).lower()
    if isinstance(exc, ConnectionError) or "refused" in message or "unreachable" in message:
        raise RuntimeError(
            f"IIS MCP endpoint is unreachable at the configured host/port: {exc}"
        ) from exc
    if isinstance(exc, urllib.error.URLError):
        raise RuntimeError(
            f"IIS MCP endpoint request failed (endpoint may be down): {exc}"
        ) from exc
    raise


class EndpointClient:
    def __init__(self, host, port, token, http=None):
        self.base = f"http://{host}:{port}"
        self.token = token
        self.http = http or UrllibHttp()

    def _headers(self):
        return {"Authorization": f"Bearer {self.token}", "Content-Type": "application/json"}

    def list_tools(self):
        try:
            status, body = self.http.get(f"{self.base}/mcp/tools/list", self._headers())
        except (urllib.error.URLError, ConnectionError) as exc:
            _translate_endpoint_error(exc)
        manifest = json.loads(body) if body else {}
        tools = []
        for entry in manifest.get("tools", []):
            tools.append({
                "name": entry.get("name", ""),
                "description": entry.get("description", ""),
                "inputSchema": entry.get("input_schema", {"type": "object"}),
            })
        return tools

    def call_tool(self, name, arguments):
        payload = json.dumps({"tool": name, "arguments": arguments or {}})
        try:
            status, body = self.http.post(f"{self.base}/mcp/tools/call", self._headers(), payload)
        except (urllib.error.URLError, ConnectionError) as exc:
            _translate_endpoint_error(exc)
        return json.loads(body) if body else {}
