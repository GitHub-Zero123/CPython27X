#pragma once

namespace Python27X {
    // 初始化补丁方法
    void InitializePatch();
    // 强制启用UTF-8编码支持
    void ForceEnableUTF8Support(bool enable);
}