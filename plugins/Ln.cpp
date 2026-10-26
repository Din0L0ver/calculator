#include "PluginSupport.h"

namespace
{
int CALC_CALL evaluate(double const* arguments, unsigned count, double* result,
                       char* error, unsigned capacity) noexcept
{
    return invokePlugin(arguments, count, 1, result, error, capacity, [](double const* a)
    {
        if (a[0] <= 0.0)
        {
            throw std::domain_error("ln requires a positive argument");
        }
        return std::log(a[0]);
    });
}
PluginInfo const info{PluginApiVersion, "ln", 1, NamedFunction, 0, 0, evaluate};
}

CALC_EXPORT PluginInfo const* CALC_CALL getPluginInfo() noexcept
{
    return &info;
}
