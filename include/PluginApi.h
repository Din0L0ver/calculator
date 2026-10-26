#pragma once

#ifdef _WIN32
#define CALC_EXPORT extern "C" __declspec(dllexport)
#define CALC_CALL __cdecl
#else
#define CALC_EXPORT extern "C" __attribute__((visibility("default")))
#define CALC_CALL
#endif

// C-совместимая граница DLL: память и исключения остаются внутри своего модуля.
constexpr unsigned PluginApiVersion = 1;
enum PluginKind : unsigned { NamedFunction = 0, BinaryOperator = 1 };

using EvaluateFunction = int (CALC_CALL *)(double const*, unsigned, double*, char*, unsigned);

struct PluginInfo
{
    unsigned version;
    char const* name;
    unsigned arity;
    unsigned kind;
    unsigned precedence;
    unsigned rightAssociative;
    EvaluateFunction evaluate;
};

using GetPluginInfo = PluginInfo const* (CALC_CALL *)();
