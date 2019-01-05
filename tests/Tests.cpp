#include "gtest/gtest.h"
#include <sstream>
#include "src/Process.h"
#include "src/SharedObject.h"
#include "src/SymbolSet.h"
#include "src/Allocator.h"

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
}

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new coreTest::CoreClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

