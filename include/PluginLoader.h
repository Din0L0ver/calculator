#pragma once
#include "Operation.h"
#include <filesystem>
#include <iosfwd>

class PluginLoader final
{
public:
    void load(std::filesystem::path const& directory, OperationRegistry& operations,
              std::ostream& diagnostics) const;
};
