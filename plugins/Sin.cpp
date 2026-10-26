#include "PluginSupport.h"

namespace
{
int CALC_CALL evaluate(double const* arguments, unsigned count, double* result,
                       char* error, unsigned capacity) noexcept
{
    return invokePlugin(arguments, count, 1, result, error, capacity, [](double const* a)
    {
        double const pi = std::acos(-1.0);
        return std::sin(std::remainder(a[0], 360.0) * pi / 180.0);
    });
}
PluginInfo const info{PluginApiVersion, "sin", 1, NamedFunction, 0, 0, evaluate};
}

CALC_EXPORT PluginInfo const* CALC_CALL getPluginInfo() noexcept
{
    return &info;
}
