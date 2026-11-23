#include "Command.h"
#include <cmath>
#include <stdexcept>

void PushCommand::execute(std::vector<double>& stack) const
{
    stack.push_back(m_value);
}

void OperationCommand::execute(std::vector<double>& stack) const
{
    auto const arity = m_operation->info().arity;
    if (stack.size() < arity)
    {
        throw std::runtime_error("Not enough operands");
    }
    auto const first = stack.end() - arity;
    std::vector<double> arguments(first, stack.end());
    double const result = m_operation->calculate(arguments);
    if (!std::isfinite(result))
    {
        throw std::runtime_error("Non-finite result: " + m_operation->info().name);
    }
    // До успешного вычисления стек не меняется.
    stack.resize(stack.size() - arity);
    stack.push_back(result);
}

void NegateCommand::execute(std::vector<double>& stack) const
{
    if (stack.empty())
    {
        throw std::runtime_error("Missing operand");
    }
    stack.back() = -stack.back();
}

double Calculator::execute(Program const& program) const
{
    std::vector<double> stack;
    stack.reserve(program.size());
    for (auto const& command : program)
    {
        command->execute(stack);
    }
    if (stack.size() != 1)
    {
        throw std::runtime_error("Invalid expression");
    }
    return stack.back();
}
