#include "gtest/gtest.h"
#include <sstream>
#include <mutex>
#include <thread>
#include <chrono>
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
    
    class CoreClassTest : public ::testing::Environment
    {
    public:
        void SetUp() override
        {
            core::Logger::Instance().Start(core::TraceSeverity::Info);
        }
    };
    
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
        
        thr_a.Start();
        thr_b.Start();
        thr_c.Start();
        thr_d.Start();
        thr_a.Join();
        thr_b.Join();
        thr_c.Join();
        thr_d.Join();
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
        
        thr_waiterA.Start();
        thr_waiterB.Start();
        thr_signal.Start();
        thr_waiterA.Join();
        thr_waiterB.Join();
        thr_signal.Join();
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
        
        thr_waiterA.Start();
        thr_waiterB.Start();
        thr_signal.Start();
        thr_waiterA.Join();
        thr_waiterB.Join();
        thr_signal.Join();
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
        
        thr_waiterA.Start();
        thr_waiterB.Start();
        thr_signal.Start();
        thr_waiterA.Join();
        thr_waiterB.Join();
        thr_signal.Join();
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
        auto executor = core::AsyncExecutor<core::ExecutionModel::Process, 6>::make_executor("Core_Test_ProcessAsyncExecutor", true);
        auto future = executor->make_task<int>([](int i){return ++i;}, 5);
        int& result = future->get();
    }
}

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new coreTest::CoreClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

