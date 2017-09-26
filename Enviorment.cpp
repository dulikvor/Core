#include "Enviorment.h"
#include <unistd.h>
#include "Assert.h"
#include "Directory.h"

using namespace std;

namespace core
{
	Enviorment& Enviorment::Instance()
	{
		static Enviorment instance;
		return instance;
	}

	void Enviorment::Init()
	{
		ASSERT(m_initiated.exchange(true) == false);
		LINUX_VERIFY((m_coreCount = sysconf(_SC_NPROCESSORS_ONLN)) != -1);
		ReadProcessLocation();
	}

	void Enviorment::ReadProcessLocation()
	{
		char buffer[MAX_WORKING_DIR_SIZE];
		ssize_t byteCount = readlink("/proc/self/exe", buffer, MAX_WORKING_DIR_SIZE);
		string processFullPath = string(buffer, byteCount > 0 ? byteCount : 0);
		m_processPath = Directory::GetDir(processFullPath);
		m_processName = Directory::GetProcessName(processFullPath);
	}
}
