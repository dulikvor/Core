#pragma once

#include <functional>
#include <condition_variable>
#include <mutex>
#include <string>
#include <tuple>
#include <memory>
#include <type_traits>
#include "NoExcept.h"
#include "Allocator.h"
#include "Exception.h"
#include "Condition.h"
#include "Mutex.h"
#include "TypeTraits.h"
#include "EnumsAll.h"

namespace core
{
    template<ExecutionModel>
    class ConcreteAsyncTask
    {
    
    };
    
    class AsyncTask
    {
    public:
        typedef std::unique_ptr<AsyncTask, std::function<void(AsyncTask*)>> task_ptr;
        typedef std::shared_ptr<AsyncTask> shared_task_ptr;
        
        enum class AsyncTaskState
        {
            CREATED,
            RUNNING,
            CANCELED,
            COMPLETED
        };
        
        virtual ~AsyncTask()=default;
        
        virtual void start() = 0;
        virtual void complete() = 0;
        virtual void change_state(AsyncTaskState newState) = 0;
        virtual void set_failure_reason(const std::string& failureReason) = 0;
        virtual AsyncTaskState get_state() const = 0;
        virtual std::string get_failure_reason() const = 0;
        virtual void wait() = 0;
        virtual void notify_on_failure() = 0;
        virtual bool is_terminate_task() const {return false;}
    };
    
    template<typename Mutex, typename ConditionVar, typename Callable, typename... Args>
    class _AsyncTask : public AsyncTask
    {
    public:
        typedef _AsyncTask _self;
        struct shared_task{};
        
        _AsyncTask()=default;
        
        template<typename _Mutex = Mutex, typename = typename std::enable_if<std::is_default_constructible<_Mutex>::value>::type,
                typename _ConditionVar = ConditionVar, typename = typename std::enable_if<std::is_default_constructible<_ConditionVar>::value>::type,
                typename... _Args>
        explicit _AsyncTask(Callable func, _Args&&... args)
            :m_state(AsyncTaskState::CREATED), m_func(func), m_args(std::forward<_Args>(args)...)
        {}
    
        _AsyncTask(_AsyncTask&& object) NOEXCEPT
            :m_state(object.m_state), m_func(std::move(object.m_func)), m_args(std::move(object.m_args)),
                m_failureReason(std::move(m_failureReason)), m_waitMut(object.m_waitMut), m_waitCv(object.m_waitMut){}
                
        ~_AsyncTask() override=default;
    
        void start() override
        {
            m_state = AsyncTaskState::RUNNING;
            start_impl(std::make_index_sequence<sizeof...(Args)>());
            m_state = AsyncTaskState::COMPLETED;
            {
                std::unique_lock<Mutex> localLock(m_waitMut);
                m_waitCv.notify_one();
            }
        }
        
        void complete() override
        {
            m_state = AsyncTaskState::COMPLETED;
            {
                std::unique_lock<Mutex> localLock(m_waitMut);
                m_waitCv.notify_one();
            }
        }
    
        void change_state(AsyncTaskState newState)  override { m_state = newState; }
        void set_failure_reason(const std::string& failureReason) override { m_failureReason = failureReason; }
        AsyncTaskState get_state() const  override { return m_state; }
        std::string get_failure_reason() const  override { return m_failureReason; }
        
        void wait() override
        {
            std::unique_lock<Mutex> localLock(m_waitMut);
            m_waitCv.wait(localLock, [this]{return m_state == AsyncTaskState::CANCELED ||
                                                m_state == AsyncTaskState::COMPLETED;});
            if(m_state == AsyncTaskState::CANCELED)
                throw Exception(__CORE_SOURCE, "%s", m_failureReason.c_str());
        }
        
        void notify_on_failure() override
        {
            std::unique_lock<Mutex> localLock(m_waitMut);
            m_state = AsyncTaskState::CANCELED;
            m_waitCv.notify_one();
        }
        
        static void* operator new(std::size_t count)
        {
            return ::operator new(count);
        }
        
        static void operator delete(void* ptr)
        {
            ::operator delete(reinterpret_cast<_self*>(ptr));
        }
        
        template<typename Allocator>
        static void* operator new(std::size_t count, Allocator& allocator)
        {
            return reinterpret_cast<void*>(allocator.allocate(1));
        }
        
        template<typename Allocator>
        static void operator delete(void* ptr, Allocator& allocator)
        {
            allocator.deallocate(reinterpret_cast<_self*>(ptr));
        }
        
    protected:
        AsyncTaskState m_state;
        Callable m_func;
        std::tuple<Args...> m_args;
        std::string m_failureReason;
        Mutex m_waitMut;
        ConditionVar m_waitCv;
        
    private:
        friend class ConcreteAsyncTask<ExecutionModel::Process>;
        friend class ConcreteAsyncTask<ExecutionModel::Thread>;
        
        template<std::size_t... idx>
        void start_impl(std::index_sequence<idx...>)
        {
            m_func(std::get<idx>(m_args)...);
        }
    
    };
    
    template<typename Mutex, typename ConditionVar, typename Callable, typename... Args>
    class _TerminateTask : public _AsyncTask<Mutex, ConditionVar, Callable, Args...>
    {
    public:
        typedef _AsyncTask<Mutex, ConditionVar, Callable, Args...> _base;
        template<typename... _Args>
        explicit _TerminateTask(Callable func, _Args&&... args)
            :_AsyncTask<Mutex, ConditionVar, Callable, Args...>::_AsyncTask(func, std::forward<_Args>(args)...)
        {}
        
        virtual ~_TerminateTask()=default;
    
        bool is_terminate_task() const override {return true;}
    
        static void* operator new(std::size_t count)
        {
            return _base::operator new(count);
        }
    
        static void operator delete(void* ptr)
        {
            _base::operator delete(ptr);
        }
    
        template<typename Allocator>
        static void* operator new(std::size_t count, Allocator& allocator)
        {
            auto _allocator = reinterpret_cast<typename Allocator::template rebind<_base>::other*>(&allocator);
            return _base::operator new(count, *_allocator);
        }
    
        template<typename Allocator>
        static void operator delete(void* ptr, Allocator& allocator)
        {
            auto _allocator = reinterpret_cast<typename Allocator::template rebind<_base>::other*>(&allocator);
            return _base::operator delete(ptr, *_allocator);
        }
    };
    
    struct terminate_task{};
    
    template<>
    class ConcreteAsyncTask<ExecutionModel::Process> : public AsyncTask
    {
    public:
        typedef ConcreteAsyncTask<ExecutionModel::Process> _self;
        typedef Mutex _mutex;
        typedef ConditionVariable _conditional_var;
        
        template<typename Callable, typename... Args, typename = typename std::enable_if<
                is_callable<Callable, Args...>::value>::type>
        ConcreteAsyncTask(Allocator<_AsyncTask<_mutex, _conditional_var, Callable, Args...>>& allocator,
                Callable func, Args&&... args)
        {
            typedef _AsyncTask<_mutex, _conditional_var, Callable, Args...> _task;
            
            m_task = AsyncTask::task_ptr(
                    new(allocator)_task(std::forward<Callable>(func), std::forward<Args>(args)...),
                    [&allocator](AsyncTask* ptr){
                        auto task = reinterpret_cast<_task*>(ptr);
                        AsyncTaskState state = task->get_state();
                        if(state == AsyncTaskState::CANCELED || state == AsyncTaskState::COMPLETED)
                            _task::operator delete(ptr, allocator);
                    }
                );
        }
    
        template<typename Callable, typename... Args, typename = typename std::enable_if<
                is_callable<Callable, Args...>::value>::type>
        ConcreteAsyncTask(terminate_task, Allocator<_TerminateTask<_mutex, _conditional_var, Callable, Args...>>& allocator,
                          Callable func, Args&&... args)
        {
            typedef _TerminateTask<_mutex, _conditional_var, Callable, Args...> _task;
        
            m_task = AsyncTask::task_ptr(
                    new(allocator)_task(std::forward<Callable>(func), std::forward<Args>(args)...),
                    [&allocator](AsyncTask* ptr){
                        auto task = reinterpret_cast<_task*>(ptr);
                        AsyncTaskState state = task->get_state();
                        if(state == AsyncTaskState::CANCELED || state == AsyncTaskState::COMPLETED)
                            _task::operator delete(ptr, allocator);
                    }
            );
        }
        
        ~ConcreteAsyncTask() override=default;
        
        void start() override { m_task->start(); }
        void complete() override { m_task->complete(); }
        void change_state(AsyncTaskState newState) override { m_task->change_state(newState); }
        void set_failure_reason(const std::string& failureReason) override { m_task->set_failure_reason(failureReason); }
        AsyncTaskState get_state() const override { return m_task->get_state(); }
        std::string get_failure_reason() const override { return m_task->get_failure_reason(); }
        void wait() override { m_task->wait(); }
        void notify_on_failure() override { m_task->notify_on_failure(); }
        
        AsyncTask* get_task() {return m_task.get();}
        
    private:
        AsyncTask::task_ptr m_task;
    };
    
    template<>
    class ConcreteAsyncTask<ExecutionModel::Thread> : public AsyncTask
    {
    public:
        typedef ConcreteAsyncTask<ExecutionModel::Thread> _self;
        
        template<typename Callable, typename... Args, typename = typename std::enable_if<
                is_callable<Callable, Args...>::value>::type>
        explicit ConcreteAsyncTask(Callable func, Args&&... args)
        {
            typedef _AsyncTask<std::mutex, std::condition_variable, Callable, Args...> _task;
            m_task.reset(new _task(std::forward<Callable>(func), std::forward<Args>(args)...));
        }
    
        template<typename Callable, typename... Args, typename = typename std::enable_if<
                is_callable<Callable, Args...>::value>::type>
        ConcreteAsyncTask(terminate_task, Callable func, Args&&... args)
        {
            typedef _TerminateTask<std::mutex, std::condition_variable, Callable, Args...> _task;
            m_task.reset(new _task(std::forward<Callable>(func), std::forward<Args>(args)...));
        }
        
        ~ConcreteAsyncTask() override=default;
    
        void start() override { m_task->start(); }
        void complete() override { m_task->complete(); }
        void change_state(AsyncTaskState newState) override { m_task->change_state(newState); }
        void set_failure_reason(const std::string& failureReason) override { m_task->set_failure_reason(failureReason); }
        AsyncTaskState get_state() const override { return m_task->get_state(); }
        std::string get_failure_reason() const override { return m_task->get_failure_reason(); }
        void wait() override { m_task->wait(); }
        void notify_on_failure() override { m_task->notify_on_failure(); }
    
        AsyncTask::shared_task_ptr get_task() {return m_task;}
        
    private:
        AsyncTask::shared_task_ptr m_task;
    };
    
    template<ExecutionModel model, typename Allocator, typename Callable, typename... Args>
    inline AsyncTask::task_ptr make_task(Allocator& allocator, Callable callable, Args&&... args)
    {
        static_assert(model == ExecutionModel::Process, "Execution model should be process");
        return AsyncTask::task_ptr(
                new ConcreteAsyncTask<ExecutionModel::Process>(
                        allocator, callable, std::forward<Args>(args)...) );
    }
    
    template<ExecutionModel model, typename Callable, typename... Args>
    inline AsyncTask::task_ptr make_task(Callable callable, Args&&... args)
    {
        static_assert(model == ExecutionModel::Thread, "Execution model should be thread");
        return AsyncTask::task_ptr(
                new ConcreteAsyncTask<ExecutionModel::Thread>(
                        callable, std::forward<Args>(args)...) );
    }
}
