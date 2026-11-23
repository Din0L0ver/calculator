#include "Operation.h"
#include <stdexcept>
#include <utility>

namespace
{
class BasicOperation final : public IOperation
{
public:
    BasicOperation(std::string name, unsigned precedence,
                   std::function<double(double, double)> function)
        : m_info{std::move(name), 2, true, precedence, false},
          m_function(std::move(function)) {}

    OperationInfo const& info() const override { return m_info; }
    double calculate(std::vector<double> const& arguments) const override
    {
        if (arguments.size() != 2)
        {
            throw std::runtime_error("Expected two arguments");
        }
        return m_function(arguments[0], arguments[1]);
    }
private:
    OperationInfo m_info;
    std::function<double(double, double)> m_function;
};
}

OperationRegistry::OperationRegistry()
{
    add(std::make_shared<BasicOperation>("+", 10, [](double a, double b) { return a + b; }));
    add(std::make_shared<BasicOperation>("-", 10, [](double a, double b) { return a - b; }));
    add(std::make_shared<BasicOperation>("*", 20, [](double a, double b) { return a * b; }));
    add(std::make_shared<BasicOperation>("/", 20, [](double a, double b)
    {
        if (b == 0.0)
        {
            throw std::domain_error("Division by zero");
        }
        return a / b;
    }));
}

void OperationRegistry::add(std::shared_ptr<IOperation const> operation)
{
    if (!operation)
    {
        throw std::invalid_argument("Null operation");
    }
    auto const name = operation->info().name;
    if (!m_operations.emplace(name, std::move(operation)).second)
    {
        throw std::runtime_error("Duplicate operation: " + name);
    }
}

std::shared_ptr<IOperation const> OperationRegistry::find(std::string const& name) const
{
    auto const it = m_operations.find(name);
    return it == m_operations.end() ? nullptr : it->second;
}
