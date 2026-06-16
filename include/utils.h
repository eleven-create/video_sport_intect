#pragma once
#include <string>

// 声明一个编码转换器：专门把 UTF-8 翻译成 Windows 认识的 GBK
std::string zh(const std::string& utf8Str);