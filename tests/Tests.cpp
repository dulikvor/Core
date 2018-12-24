#include "gtest/gtest.h"
#include "src/Process.h"
#include "src/SharedObject.h"

namespace coreTest
{
    
    class CoreClassTest : public ::testing::Environment
    {
    public:
        CoreClassTest() {}
        
        void SetUp() override
        {
            core::Logger::Instance().Start(core::TraceSeverity::Info);
        }
        void TearDown() override {}
    };
    
    TEST(Core, SharedMemory)
    {
        core::SharedObject object("Core_Test", core::SharedObject::AccessMod::READ_WRITE);
    }
}

int main(int argc, char **argv)
{
    ::testing::AddGlobalTestEnvironment(new coreTest::CoreClassTest);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

