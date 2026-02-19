#pragma once

// ==========================
// 最初基于字符串替换的方案, 此处内容已过时
// ==========================
namespace Python27X {
    // 初始化补丁方法
    void InitializePatch();
    // 强制启用UTF-8编码支持
    void ForceEnableUTF8Support(bool enable);
}