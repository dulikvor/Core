#pragma once

#include <memory>
#include <functional>
#include <thread>
#include <string>
#include <atomic>

const int MAX_THREAD_NAME = 30;

namespace core
{
    /*Thread reperesent a thread, providing the following capabilties:
    1)Creating a new thread.
    2)Starting the newly created thread at a requested function.
    3) Kill the thread.*/
    class Thread
    {
    public:
        /*Thread constrcutor will receive all needed threads states from outside (after this point the thread is immutable),
        including - the name of the thread and its starting point.*/
        Thread(const std::string& threadName, const std::function<void(void)>& requestedPoint);
        ~Thread(){} //Destructor
        Thread() = delete;
        Thread(const Thread&) = delete;
        void operator = (const Thread&) = delete;
        //Accessors
        std::thread::id GetThreadID() const {return m_thread->get_id();}
        //Start will create the thread, initiating its run.
        void Start();

        void Join() const;

    private:
        /*A deteministic entry point to the thread, providing the capability to initiate its desire entry point (the one
        received at the constructor) and to catch various exceptions (it will be best if this will be the only catch
        in the scope of the entire all the stack frames from this point.*/
        void EntryPoint(void);
        //Cleanup is a specific cancallation handler.
        static void Cleanup(void*);

    private:
        std::string m_name;
        std::function<void(void)> m_requestedPoint;
        std::unique_ptr<std::thread> m_thread;
    };
}
