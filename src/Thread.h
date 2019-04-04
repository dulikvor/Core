#pragma once

#include <stdarg.h>
#include <cassert>
#include <memory>
#include <functional>
#include <thread>
#include <string>
#include <atomic>
#include "Assert.h"
#include "NoExcept.h"

const int MAX_THREAD_NAME = 30;

namespace core
{
    class Impl_Base
    {
    public:
        virtual ~Impl_Base() NOEXCEPT(true)=default;
        virtual void join() const = 0;
        virtual std::thread::id get_id() const = 0;
    };
    
    template<typename Callable>
    class Thread_Impl : public Impl_Base
    {
    public:
        typedef std::unique_ptr<std::thread> thread_ptr;
        
        Thread_Impl(const std::string& name, Callable func)
            :m_name(name), m_func(func){
            m_thread.reset(new std::thread(std::bind(&Thread_Impl::entry_point, this)));
        }
        Thread_Impl()=delete;
        Thread_Impl(const Thread_Impl&)=delete;
        Thread_Impl& operator=(const Thread_Impl&)=delete;
        ~Thread_Impl() NOEXCEPT(true) override = default;
        
        void join() const override { m_thread->join(); }
        std::thread::id get_id() const override { return m_thread->get_id(); }
        
        void entry_point()
        {
            try{
                m_func();
            }
            catch(const Exception&){}
            catch(std::exception& e){
                TRACE_ERROR("%s", e.what());
            }
        }
    
    private:
        thread_ptr m_thread;
        std::string m_name;
        Callable m_func;
    };
    class Thread
    {
    public:
        template<typename Callable>
        Thread(const std::string& name, Callable func)
        {
            m_impl.reset(new Thread_Impl<Callable>(name, func));
        }
        ~Thread()=default;
        Thread() = delete;
        Thread(const Thread&) = delete;
        void operator = (const Thread&) = delete;
        std::thread::id get_id() const {return m_impl->get_id();}
        void join() const {m_impl->join();};

    private:
        std::unique_ptr<Impl_Base> m_impl;
    };
}
