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
template<typename Function>
int invokePlugin(double const* arguments, unsigned count, unsigned expected, double* result,
                 char* error, unsigned capacity, Function function) noexcept
{
    try
    {
        if (!result || count != expected || (count != 0 && !arguments))
        {
            throw std::invalid_argument("Invalid arguments");
        }
        double const value = function(arguments);
        if (!std::isfinite(value))
        {
            throw std::domain_error("Result is not a finite real number");
        }
        *result = value;
        return 0;
    }
    catch (std::exception const& exception)
    {
        copyError(error, capacity, exception.what());
    }
    catch (...)
    {
        copyError(error, capacity, "Unknown plugin exception");
    }
    return 1;
}
