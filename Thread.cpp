#include "Thread.h"
#include <iostream>
#include "Assert.h"
#include "Logger.h"

using namespace std;

namespace core
{
	Thread::Thread(const string& threadName, const function<void(void)>& requestedPoint)
		: m_name(threadName), m_requestedPoint(requestedPoint)
	{	
		ASSERT((int)m_name.size() <= MAX_THREAD_NAME);
	}

	void Thread::Start()
	{
		m_thread.reset(new thread(bind(&Thread::EntryPoint, this)));
	}

	void Thread::Join() const
	{
		m_thread->join();
	}

	void Thread::EntryPoint()
	{
		try
		{
			m_requestedPoint();
		}
		catch(const Exception& e)
		{
		}
		catch(std::exception& e)
		{
			TRACE_ERROR("%s", e.what());
		}
	}
}
