#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct OperationInfo
{
    std::string name;
    unsigned arity;
    bool binary;
    unsigned precedence;
    bool rightAssociative;
};

class IOperation
{
public:
    virtual ~IOperation() = default;
    virtual OperationInfo const& info() const = 0;
    virtual double calculate(std::vector<double> const& arguments) const = 0;
};

class IOperationResolver
{
public:
    virtual ~IOperationResolver() = default;
    virtual std::shared_ptr<IOperation const> find(std::string const& name) const = 0;
};

class OperationRegistry final : public IOperationResolver
{
public:
    OperationRegistry();
    void add(std::shared_ptr<IOperation const> operation);
    std::shared_ptr<IOperation const> find(std::string const& name) const override;
private:
    std::map<std::string, std::shared_ptr<IOperation const>> m_operations;
};
