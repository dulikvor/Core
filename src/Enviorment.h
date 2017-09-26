#pragma once

#include <atomic>
#include <string>

namespace core
{
	class Enviorment
	{
	public:
		static Enviorment& Instance();
		void Init();
		//Accessors
		int GetCoreCount() const { return m_coreCount; }
		const std::string& GetProcessPath() const { return m_processPath; }
		const std::string& GetProcessName() const { return m_processName; }

	private:
		void ReadProcessLocation();
	
	private:
		const int MAX_WORKING_DIR_SIZE = 500;
		int m_coreCount;
		std::string m_processPath;
		std::string m_processName;
		std::atomic_bool m_initiated;
	};
}
