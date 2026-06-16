#include "utils.h"
#include <windows.h> // 必须呼叫 Windows 底层 API

std::string zh(const std::string& utf8Str) {
    // 1. 先把 UTF-8 翻译成宽字符 (UTF-16)
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (len == 0) return utf8Str;
    std::wstring wstr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], len);

    // 2. 再把宽字符翻译成 Windows 认识的 GBK (CP_ACP)
    len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) return utf8Str;
    std::string str(len, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], len,NULL,NULL);

    // 清理一下结尾的空白符
    if (!str.empty() && str.back() == '\0') str.pop_back();

    return str;
}