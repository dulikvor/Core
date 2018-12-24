#include "gtest/gtest.h"
#include <sstream>
#include "src/Process.h"
#include "src/SharedObject.h"

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
        };
        core::ChildProcess child = core::Process::SpawnChildProcess(func);
        ::sleep(1);
        core::SharedRegion region = object.Map(8188, 5, core::SharedObject::AccessMod::READ_WRITE);//Will cross page boundries
        std::stringstream ss;
        for(int index = 0; index < region.GetSize(); index++)
            ss << region.GetPtr()[index];
    
        ASSERT_EQ(ss.str(), "Hello");
        object.Unlink();
    }
}

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new coreTest::CoreClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

