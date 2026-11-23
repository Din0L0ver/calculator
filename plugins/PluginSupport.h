#pragma once
#include "PluginApi.h"
#include <cmath>
#include <exception>
#include <stdexcept>

inline void copyError(char* buffer, unsigned capacity, char const* message) noexcept
{
    if (!buffer || capacity == 0)
    {
        return;
    }
    unsigned i = 0;
    for (; i + 1 < capacity && message[i] != '\0'; ++i)
    {
        buffer[i] = message[i];
    }
    buffer[i] = '\0';
}

// Обёртка ловит исключение до выхода из DLL, включая исключения не из std::exception.
struct PluginCall
{
    double const* arguments;
    unsigned count;
    double* result;
    char* error;
    unsigned capacity;
};

template<typename Function>
int invokePlugin(PluginCall call, unsigned expected, Function function) noexcept
{
    try
    {
        if (!call.result || call.count != expected || (call.count != 0 && !call.arguments))
        {
            throw std::invalid_argument("Invalid arguments");
        }
        double const value = function(call.arguments);
        if (!std::isfinite(value))
        {
            throw std::domain_error("Result is not a finite real number");
        }
        *call.result = value;
        return 0;
    }
    catch (std::exception const& exception)
    {
        copyError(call.error, call.capacity, exception.what());
    }
    catch (...)
    {
        copyError(call.error, call.capacity, "Unknown plugin exception");
    }
    return 1;
}
