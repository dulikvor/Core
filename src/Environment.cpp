#include "Environment.h"
#include <functional>
#include <sys/types.h>
#if defined(__linux)
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#endif
#include "Assert.h"
#include "Directory.h"

using namespace std;

namespace core
{
    Environment& Environment::Instance()
    {
        static Environment instance;
        return instance;
    }

    void Environment::Init()
    {
        if(m_initiated.exchange(true) == true)
            return;
#if defined(__linux)
        PLATFORM_VERIFY((m_coreCount = sysconf(_SC_NPROCESSORS_ONLN)) != -1);
#endif
        ReadProcessLocation();
        ReadIPV4Addresses();
    }

    void Environment::ReadProcessLocation()
    {
#if defined(__linux)
        char buffer[MAX_WORKING_DIR_SIZE];
        ssize_t byteCount = readlink("/proc/self/exe", buffer, MAX_WORKING_DIR_SIZE);
        string processFullPath = string(buffer, byteCount > 0 ? byteCount : 0);
        m_processPath = Directory::GetDirctoryFullPath(processFullPath);
        m_processName = Directory::GetFileName(processFullPath);
#endif
    }

    void Environment::ReadIPV4Addresses()
    {
#if defined(__linux)
        struct ifaddrs *ifaddr;
        PLATFORM_VERIFY(getifaddrs(&ifaddr) != -1);
        std::unique_ptr<struct ifaddrs, std::function<void(struct ifaddrs*)>> guard(ifaddr,
            [](struct ifaddrs* ptr){freeifaddrs(ptr);});
        struct ifaddrs* ifaCurrent = ifaddr;
        char host[INET_ADDRSTRLEN];
        while(ifaCurrent != NULL)
        {
            if(ifaCurrent->ifa_addr != NULL && ifaCurrent->ifa_addr->sa_family == AF_INET)
            {
                // void* addrPtr =&((struct sockaddr_in *)ifaCurrent->ifa_addr)->sin_addr;
                PLATFORM_VERIFY(getnameinfo(ifaCurrent->ifa_addr, sizeof(struct sockaddr_in), host, INET_ADDRSTRLEN,
                            NULL, 0, NI_NUMERICHOST) == 0);
                m_ipv4Adrresses.emplace_back(host);
            }
            ifaCurrent = ifaCurrent->ifa_next;
        }
#else
        m_ipv4Adrresses.emplace_back("127.0.0.1");
#endif
    }
}
