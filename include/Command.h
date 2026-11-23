#pragma once
#include "Operation.h"
#include <utility>

class ICommand
{
public:
    virtual ~ICommand() = default;
    virtual void execute(std::vector<double>& stack) const = 0;
};

using Program = std::vector<std::unique_ptr<ICommand>>;

class PushCommand final : public ICommand
{
public:
    explicit PushCommand(double value) : m_value(value) {}
    void execute(std::vector<double>& stack) const override;
private:
    double m_value;
};

class OperationCommand final : public ICommand
{
public:
    explicit OperationCommand(std::shared_ptr<IOperation const> operation)
        : m_operation(std::move(operation)) {}
    void execute(std::vector<double>& stack) const override;
private:
    std::shared_ptr<IOperation const> m_operation;
};

class NegateCommand final : public ICommand
{
public:
    void execute(std::vector<double>& stack) const override;
};

class Calculator final
{
public:
    double execute(Program const& program) const;
};
