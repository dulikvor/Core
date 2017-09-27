#include "AutoResetEvent.h"

using namespace std;

namespace core
{
    void AutoResetEvent::Set() {
        unique_lock<mutex> lock(m_mutex);
        m_set = true;
        m_conditionalVariable.notify_one();
    }

    void AutoResetEvent::WaitOne() {
        unique_lock<mutex> lock(m_mutex);
        m_conditionalVariable.wait(lock, [this]{return m_set;});
        m_set = false;
    }
}
