#include "gtest/gtest.h"
#include <sstream>
#include <mutex>
#include <thread>
#include <chrono>
#include "src/Param.h"
#include "src/Process.h"
#include "src/SharedObject.h"
#include "src/SymbolSet.h"
#include "src/Allocator.h"
#include "src/Mutex.h"
#include "src/Thread.h"
#include "src/Condition.h"
#include "src/SyncSharedQueue.h"
#include "src/AsyncTask.h"
#include "src/AsyncExecutor.h"

using namespace std::literals::chrono_literals;

namespace coreTest
{
    
    struct A
    {
    public:
        bool operator==(const A& rhs) const {return val == rhs.val;}
        int val = 5;
    };
    
    class CoreClassTest : public ::testing::Environment
    {
    public:
        void SetUp() override
        {
            core::Logger::Instance().Start(core::TraceSeverity::Info);
        }
    };
    
    TEST(Core, Params)
    {
        int lvalue_int = 5;
        auto lvalue_int_param = core::MakeParam(lvalue_int);
        core::TypedParam<int>& lvalue_int_typed_param = reinterpret_cast<core::TypedParam<int>&>(*lvalue_int_param);
        ASSERT_EQ(lvalue_int_typed_param.Get<int>(), lvalue_int);
    
        auto rvalue_int_param = core::MakeParam(5);
        core::TypedParam<int>& rvalue_int_typed_param = reinterpret_cast<core::TypedParam<int>&>(*rvalue_int_param);
        ASSERT_EQ(rvalue_int_typed_param.Get<int>(), 5);
        
        const char* lvalue_literal = "literal string";
        auto lvalue_literal_param = core::MakeParam(lvalue_literal);
        core::TypedParam<char*>& lvalue_literal_typed_param = reinterpret_cast<core::TypedParam<char*>&>(*lvalue_literal_param);
        ASSERT_EQ(std::string(lvalue_literal_typed_param.Get<char*>()), lvalue_literal);
    
        auto rvalue_literal_param = core::MakeParam("literal string");
        core::TypedParam<char*>& rvalue_literal_typed_param = reinterpret_cast<core::TypedParam<char*>&>(*rvalue_literal_param);
        ASSERT_EQ(std::string(rvalue_literal_typed_param.Get<char*>()), "literal string");
        
        A lvalue_usertype;
        auto lvalue_usertype_param = core::MakeParam(lvalue_usertype);
        core::TypedParam<A>& lvalue_usertype_typed_param = reinterpret_cast<core::TypedParam<A>&>(*lvalue_usertype_param);
        ASSERT_EQ(lvalue_usertype_typed_param.Get<A>().val, 5);
    
        auto rvalue_usertype_param = core::MakeParam(A());
        core::TypedParam<A>& rvalue_usertype_typed_param = reinterpret_cast<core::TypedParam<A>&>(*rvalue_usertype_param);
        ASSERT_EQ(rvalue_usertype_typed_param.Get<A>().val, 5);
        
        core::Param::Param_Ptr cloned_param =  lvalue_usertype_param->Clone();
        core::TypedParam<A>& cloned_typed_param = reinterpret_cast<core::TypedParam<A>&>(*cloned_param);
        ASSERT_EQ(lvalue_usertype_typed_param.Get<A>().val, cloned_typed_param.Get<A>().val);
    }
    
    TEST(Core, SharedMemory)
    {
        core::SharedObject object("Core_Test", core::SharedObject::AccessMod::READ_WRITE);
        object.Allocate(10000);
        std::function<void(void)> func = []{
            core::SharedObject object("Core_Test", core::SharedObject::AccessMod::READ_WRITE);
            core::SharedRegion region = object.Map(8188, 5, core::SharedObject::AccessMod::READ_WRITE);//Will cross page boundries
            memcpy(region.GetPtr(), "Hello", region.GetSize());
            region.UnMap();
        };
        core::ChildProcess child = core::Process::SpawnChildProcess(func);
        ::sleep(1);
        core::SharedRegion region = object.Map(8188, 5, core::SharedObject::AccessMod::READ_WRITE);//Will cross page boundries
        std::stringstream ss;
        for(int index = 0; index < region.GetSize(); index++)
            ss << region.GetPtr()[index];
    
        ASSERT_EQ(ss.str(), "Hello");
        region.UnMap();
        object.Unlink();
    }
    
    TEST(Core, SymbolSet)
    {
        core::Symbolset<3> symbolset(4, 5);
        ASSERT_EQ((int)symbolset[2], 5);
        core::Symbolset<2> symbolset_2(15, 0);
        symbolset_2[14] = 2;
        ASSERT_EQ((int)symbolset_2[14], 2);
    }
    
    TEST(Core, Allocator)
    {
        core::Allocator<char> allocator(core::HeapType::Shared, "Core_Allocator_Test", 8);
        char* ptr = allocator.allocate(2);
        char* ptr_2 = allocator.allocate(4);
        char* ptr_3 = allocator.allocate(1);
        allocator.deallocate(ptr);
        allocator.deallocate(ptr_2);
        allocator.deallocate(ptr_3);
        ptr = allocator.allocate(8);
        allocator.deallocate(ptr);
    }
    
    TEST(Core, MutexSimple)
    {
        core::Mutex mutex;
        
        auto func = [&mutex]{
            std::lock_guard<core::Mutex> guard(mutex);
            volatile int val;
            for(int idx = 0; idx < 500000; idx++)
            {
                val = idx * idx;
            }
        };
        
        core::Thread thr_a("Thread A", func);
        core::Thread thr_b("Thread B", func);
        core::Thread thr_c("Thread C", func);
        core::Thread thr_d("Thread D", func);
        
        thr_a.join();
        thr_b.join();
        thr_c.join();
        thr_d.join();
    }
    
    TEST(Core, ConditionSimple)
    {
        core::Mutex mutex;
        core::Condition condition;
        auto wait_f = [&mutex, &condition]{
            std::unique_lock<core::Mutex> lock(mutex);
            condition.wait(lock);
        };
    
        core::Thread thr_waiterA("Waiter A", wait_f);
        core::Thread thr_waiterB("Waiter B", wait_f);
        core::Thread thr_signal("Signal", [&mutex, &condition]{
            for(int idx = 0; idx < 2; idx++)
            {
                std::this_thread::sleep_for(100ms);
                std::unique_lock<core::Mutex> guard(mutex);
                condition.signal(core::Condition::NOTIFY_ONE);
            }
        });
        
        thr_waiterA.join();
        thr_waiterB.join();
        thr_signal.join();
    }
    
    TEST(Core, ConditionNotifyAll)
    {
        core::Mutex mutex;
        core::Condition condition;
        auto wait_f = [&mutex, &condition]{
            std::unique_lock<core::Mutex> lock(mutex);
            condition.wait(lock);
        };
        
        core::Thread thr_waiterA("Waiter A", wait_f);
        core::Thread thr_waiterB("Waiter B", wait_f);
        core::Thread thr_signal("Signal", [&mutex, &condition]{
                std::this_thread::sleep_for(100ms);
                std::lock_guard<core::Mutex> guard(mutex);
                condition.signal(core::Condition::NOTIFY_ALL);
        });
        
        thr_waiterA.join();
        thr_waiterB.join();
        thr_signal.join();
    }
    
    TEST(Core, ConditionVariableNotifyAll)
    {
        core::Mutex mutex;
        core::ConditionVariable cv;
        bool signal = false;
        auto wait_f = [&mutex, &cv, &signal]{
            std::unique_lock<core::Mutex> lock(mutex);
            cv.wait(lock, [&signal]{ return signal; });
        };
        
        core::Thread thr_waiterA("Waiter A", wait_f);
        core::Thread thr_waiterB("Waiter B", wait_f);
        core::Thread thr_signal("Signal", [&mutex, &cv, &signal]{
            std::this_thread::sleep_for(100ms);
            std::lock_guard<core::Mutex> guard(mutex);
            signal = true;
            cv.notify_all();
        });
        
        thr_waiterA.join();
        thr_waiterB.join();
        thr_signal.join();
    }
    
    TEST(Core, SyncSharedQueue)
    {
        std::function<void(void)> func = []{
            ::sleep(1);
            core::SyncSharedQueue<int, 10> queue("Core_Test_SyncSharedQueue", false, core::SharedObject::AccessMod::READ_WRITE);
            for(int idx = 10000; idx >= 0; idx--)
            {
                queue.push(idx);
            }
        };
        
        core::ChildProcess process = core::Process::SpawnChildProcess(func);
        core::SyncSharedQueue<int, 10> queue("Core_Test_SyncSharedQueue", true, core::SharedObject::AccessMod::READ_WRITE);
        int item = 1;
        while(item)
        {
            queue.pop(item);
        }
    }
    
    TEST(Core, ProcessAsyncExecutor)
    {
        auto executor = core::AsyncExecutor<core::ExecutionModel::Process, 10>::make_executor("Core_Test_ProcessAsyncExecutor", true);
        std::vector<core::future<int>> futures;
        for(int idx = 0; idx < 1000; idx++)
        {
            futures.emplace_back(executor->make_task<int>([](int i){return ++i;}, idx));
        }
        
        for(auto& future : futures)
        {
            static int futureIdx = 0;
            ASSERT_EQ(future->get(), futureIdx + 1);
            futureIdx++;
        }
    }
}

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new coreTest::CoreClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

