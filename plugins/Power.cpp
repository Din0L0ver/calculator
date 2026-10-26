#include "PluginSupport.h"

namespace
{
int CALC_CALL evaluate(double const* arguments, unsigned count, double* result,
                       char* error, unsigned capacity) noexcept
{
    return invokePlugin(arguments, count, 2, result, error, capacity, [](double const* a)
    {
        if (a[0] == 0.0 && a[1] <= 0.0)
        {
            throw std::domain_error("Zero cannot be raised to a non-positive power");
        }
        return std::pow(a[0], a[1]);
    });
}
PluginInfo const info{PluginApiVersion, "^", 2, BinaryOperator, 40, 1, evaluate};
}

CALC_EXPORT PluginInfo const* CALC_CALL getPluginInfo() noexcept
{
    return &info;
}
