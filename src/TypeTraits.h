#pragma once

#include <type_traits>
#include <functional>

namespace core{
    template<typename Callable, typename... Args>
    struct is_callable : std::is_constructible<std::function<void(Args...)>, Callable>{};
    
    template<typename Callable, typename Ret, typename... Args>
    struct is_callable_ret : std::is_constructible<std::function<Ret(Args...)>, Callable>{};
}