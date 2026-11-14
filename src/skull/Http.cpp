#include "skull/Http.h"

#include <windows.h>
#include <winhttp.h>

#include <string>
#include <vector>

namespace custom_skull {

namespace {

std::wstring toWide(std::string const& text) {
    if (text.empty()) {
        return {};
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size);
    return result;
}

struct InternetHandle {
    HINTERNET handle = nullptr;
    explicit InternetHandle(HINTERNET value) : handle(value) {}
    ~InternetHandle() {
        if (handle) {
            WinHttpCloseHandle(handle);
        }
    }
    InternetHandle()                             = default;
    InternetHandle(InternetHandle const&)        = delete;
    InternetHandle& operator=(InternetHandle const&) = delete;
    InternetHandle(InternetHandle&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
};

std::string lastErrorText(char const* stage) {
    return std::string(stage) + " (WinHTTP error " + std::to_string(GetLastError()) + ")";
}

} // namespace

std::optional<std::string> httpGet(std::string const& url, std::string& error, int timeoutMs) {
    std::wstring wideUrl = toWide(url);

    URL_COMPONENTS components{};
    components.dwStructSize      = sizeof(components);
    components.dwSchemeLength    = static_cast<DWORD>(-1);
    components.dwHostNameLength  = static_cast<DWORD>(-1);
    components.dwUrlPathLength   = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wideUrl.c_str(), static_cast<DWORD>(wideUrl.size()), 0, &components)) {
        error = "invalid URL: " + url;
        return std::nullopt;
    }

    bool const secure = components.nScheme == INTERNET_SCHEME_HTTPS;
    if (!secure && components.nScheme != INTERNET_SCHEME_HTTP) {
        error = "unsupported URL scheme";
        return std::nullopt;
    }

    std::wstring host(components.lpszHostName, components.dwHostNameLength);
    std::wstring path(components.lpszUrlPath, components.dwUrlPathLength);
    if (components.dwExtraInfoLength > 0) {
        path.append(components.lpszExtraInfo, components.dwExtraInfoLength);
    }
    if (path.empty()) {
        path = L"/";
    }

    InternetHandle session{WinHttpOpen(
        L"CustomSkull/1.0",
        WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    )};
    if (!session.handle) {
        error = lastErrorText("WinHttpOpen");
        return std::nullopt;
    }
    WinHttpSetTimeouts(session.handle, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    InternetHandle connection{WinHttpConnect(session.handle, host.c_str(), components.nPort, 0)};
    if (!connection.handle) {
        error = lastErrorText("WinHttpConnect");
        return std::nullopt;
    }

    InternetHandle request{WinHttpOpenRequest(
        connection.handle,
        L"GET",
        path.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        secure ? WINHTTP_FLAG_SECURE : 0
    )};
    if (!request.handle) {
        error = lastErrorText("WinHttpOpenRequest");
        return std::nullopt;
    }

    DWORD redirectPolicy = WINHTTP_OPTION_REDIRECT_POLICY_ALWAYS;
    WinHttpSetOption(request.handle, WINHTTP_OPTION_REDIRECT_POLICY, &redirectPolicy, sizeof(redirectPolicy));
    DWORD decompression = WINHTTP_DECOMPRESSION_FLAG_ALL;
    WinHttpSetOption(request.handle, WINHTTP_OPTION_DECOMPRESSION, &decompression, sizeof(decompression));

    if (!WinHttpSendRequest(
            request.handle,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0,
            WINHTTP_NO_REQUEST_DATA,
            0,
            0,
            0
        )) {
        error = lastErrorText("WinHttpSendRequest");
        return std::nullopt;
    }
    if (!WinHttpReceiveResponse(request.handle, nullptr)) {
        error = lastErrorText("WinHttpReceiveResponse");
        return std::nullopt;
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (!WinHttpQueryHeaders(
            request.handle,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &statusCode,
            &statusSize,
            WINHTTP_NO_HEADER_INDEX
        )) {
        error = lastErrorText("WinHttpQueryHeaders");
        return std::nullopt;
    }
    if (statusCode < 200 || statusCode >= 300) {
        error = "HTTP " + std::to_string(statusCode);
        return std::nullopt;
    }

    std::string body;
    for (;;) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request.handle, &available)) {
            error = lastErrorText("WinHttpQueryDataAvailable");
            return std::nullopt;
        }
        if (available == 0) {
            break;
        }
        std::vector<char> chunk(available);
        DWORD             read = 0;
        if (!WinHttpReadData(request.handle, chunk.data(), available, &read)) {
            error = lastErrorText("WinHttpReadData");
            return std::nullopt;
        }
        if (read == 0) {
            break;
        }
        body.append(chunk.data(), read);
        if (body.size() > 32 * 1024 * 1024) {
            error = "response too large";
            return std::nullopt;
        }
    }
    return body;
}

} // namespace custom_skull
