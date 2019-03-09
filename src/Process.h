#pragma once
#include <vector>
#include <type_traits>
#include <functional>
#if defined(__linux)
#include <unistd.h>
#endif
#include "Assert.h"
#include "Pipe.h"
#include "ChildProcess.h"
#include "TypeTraits.h"

namespace core
{
    class Process {
    private:
        template<typename... Args>
        struct Comperator
        {
            static const bool value = sizeof...(Args) == 0;
        };
    public:
        template<typename... Args>
        static ChildProcess SpawnChildProcess(const char* processPath, Args... args){
            ValidateArgsType<Args...>();

            std::unique_ptr<Pipe> stdOutPipe(new Pipe());
            std::unique_ptr<Pipe> stdErrorPipe(new Pipe());
            pid_t processID = fork();
            PLATFORM_VERIFY(processID != -1);
            if(processID == 0) { //child
                stdOutPipe->CloseReadDescriptor();
                dup2(stdOutPipe->GetWriteDescriptor(), Pipe::STD_OUT);
                stdErrorPipe->CloseReadDescriptor();
                dup2(stdErrorPipe->GetWriteDescriptor(), Pipe::STD_ERROR);
                execlp(processPath, args...);
                exit(0);
            }
            //Only parent will reach this part
            return ChildProcess(processID, stdOutPipe, stdErrorPipe);
        }
    
        template<typename Callable, typename... Params, typename... Args>
        static auto SpawnChildProcess(const Callable& function, Args&&... args) ->
            typename std::enable_if<is_callable<Callable, Args...>::value, ChildProcess>::type
        {
            std::unique_ptr<Pipe> stdOutPipe(new Pipe());
            std::unique_ptr<Pipe> stdErrorPipe(new Pipe());
            pid_t processID = fork();
            PLATFORM_VERIFY(processID != -1);
            if(processID == 0) { //child
                stdOutPipe->CloseReadDescriptor();
                dup2(stdOutPipe->GetWriteDescriptor(), Pipe::STD_OUT);
                stdErrorPipe->CloseReadDescriptor();
                dup2(stdErrorPipe->GetWriteDescriptor(), Pipe::STD_ERROR);
                function(std::forward<Args>(args)...);
                exit(0);
            }
            //Only parent will reach this part
            return ChildProcess(processID, stdOutPipe, stdErrorPipe);
        }

    private:
        template<typename... Args>
        static typename std::enable_if<Comperator<Args...>::value>::type ValidateArgsType(){}
        template<typename First, typename... Args>
        static void ValidateArgsType(){
            static_assert(std::is_same<First, const char*>::value == true, "Format only supports c-type string as type");
            ValidateArgsType<Args...>();
        }
    };
}
