#pragma once

#include <atomic>
#include <string>
#include <vector>
#include "Export.h"

namespace core
{
    class Environment
    {
    public:
        CORE_EXPORT static Environment& Instance();
        CORE_EXPORT void Init();
        //Accessors
        int GetCoreCount() const { return m_coreCount; }
        const std::string& GetProcessPath() const { return m_processPath; }
        const std::string& GetProcessName() const { return m_processName; }
        const std::vector<std::string> GetIPV4Addresses() const{ return m_ipv4Adrresses;}

    private:
        void ReadProcessLocation();
        void ReadIPV4Addresses();
    
    private:
        static const int MAX_WORKING_DIR_SIZE = 500;
        int m_coreCount;
        std::string m_processPath;
        std::string m_processName;
        std::atomic_bool m_initiated;
        std::vector<std::string> m_ipv4Adrresses;
    };
}
