import json
import pytest
from iis_mcp_proxy.client import EndpointClient


class FakeHttp:
    def __init__(self, get_body, post_body, status=200):
        self.get_body = get_body
        self.post_body = post_body
        self.status = status
        self.last_post = None

    def get(self, url, headers):
        return self.status, self.get_body

    def post(self, url, headers, body):
        self.last_post = (url, headers, body)
        return self.status, self.post_body


def test_list_tools_maps_manifest():
    manifest = json.dumps({
        "tools": [
            {"name": "iis_search", "description": "search", "input_schema": {"type": "object"}}
        ]
    })
    http = FakeHttp(get_body=manifest, post_body="{}")
    client = EndpointClient("127.0.0.1", 8731, "tok", http=http)
    tools = client.list_tools()
    assert len(tools) == 1
    assert tools[0]["name"] == "iis_search"
    assert tools[0]["description"] == "search"
    assert tools[0]["inputSchema"] == {"type": "object"}


def test_call_tool_posts_tool_and_arguments():
    http = FakeHttp(get_body="{}", post_body=json.dumps({"status": "Ready"}))
    client = EndpointClient("127.0.0.1", 8731, "tok", http=http)
    result = client.call_tool("iis_search", {"query_text": "spline"})
    url, headers, body = http.last_post
    sent = json.loads(body)
    assert sent == {"tool": "iis_search", "arguments": {"query_text": "spline"}}
    assert headers["Authorization"] == "Bearer tok"
    assert result == {"status": "Ready"}


class FailingHttp:
    def get(self, url, headers):
        raise ConnectionError("refused")

    def post(self, url, headers, body):
        raise ConnectionError("refused")


def test_call_tool_endpoint_down_raises_clear_error():
    client = EndpointClient("127.0.0.1", 8731, "tok", http=FailingHttp())
    with pytest.raises(RuntimeError) as exc_info:
        client.call_tool("iis_search", {"query_text": "x"})
    message = str(exc_info.value).lower()
    assert "endpoint" in message or "refused" in message or "unreachable" in message
