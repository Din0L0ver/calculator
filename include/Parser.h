#pragma once
#include "Command.h"

class Parser final
{
public:
    explicit Parser(IOperationResolver const& operations) : m_operations(operations) {}
    Program parse(std::string const& expression) const;
private:
    IOperationResolver const& m_operations;
};
