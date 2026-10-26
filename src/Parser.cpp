#include "Parser.h"
#include <cctype>
#include <cmath>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace
{
bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isLetter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Состояние разбора живёт только в пределах одного вызова parse.
class ParseState final
{
public:
    ParseState(std::string const& text, IOperationResolver const& operations)
        : m_text(text), m_operations(operations) {}

    Program parse()
    {
        expression(0, 0);
        if (peek() != '\0')
        {
            fail("Unexpected token");
        }
        return std::move(m_program);
    }
private:
    char peek()
    {
        while (m_position < m_text.size() &&
               std::isspace(static_cast<unsigned char>(m_text[m_position])))
        {
            ++m_position;
        }
        return m_position == m_text.size() ? '\0' : m_text[m_position];
    }

    [[noreturn]] void fail(std::string const& message) const
    {
        throw std::runtime_error(message + " at position " + std::to_string(m_position + 1));
    }

    void expression(unsigned minimum, unsigned depth)
    {
        if (depth > 256)
        {
            fail("Expression nesting is too deep");
        }
        primary(depth);
        while (true)
        {
            char const c = peek();
            auto const operation = m_operations.find(std::string(1, c));
            if (!operation || !operation->info().binary || operation->info().precedence < minimum)
            {
                break;
            }
            ++m_position;
            auto const& info = operation->info();
            expression(info.precedence + (info.rightAssociative ? 0 : 1), depth + 1);
            m_program.push_back(std::make_unique<OperationCommand>(operation));
        }
    }

    void primary(unsigned depth)
    {
        char const c = peek();
        if (c == '+' || c == '-')
        {
            ++m_position;
            // Степень (40) сильнее унарного минуса (30): -2^2 == -4.
            expression(30, depth + 1);
            if (c == '-')
            {
                m_program.push_back(std::make_unique<NegateCommand>());
            }
        }
        else if (c == '(')
        {
            ++m_position;
            expression(0, depth + 1);
            require(')');
        }
        else if (isDigit(c) || c == '.')
        {
            number();
        }
        else if (isLetter(c))
        {
            function(depth);
        }
        else
        {
            fail("Expected a number, function or '('");
        }
    }

    void require(char c)
    {
        if (peek() != c)
        {
            fail(std::string("Expected '") + c + "'");
        }
        ++m_position;
    }

    void number()
    {
        auto const start = m_position;
        bool digits = false;
        auto scanDigits = [&]()
        {
            while (m_position < m_text.size() && isDigit(m_text[m_position]))
            {
                digits = true;
                ++m_position;
            }
        };
        scanDigits();
        if (m_position < m_text.size() && m_text[m_position] == '.')
        {
            ++m_position;
            scanDigits();
        }
        if (!digits)
        {
            fail("Invalid number");
        }
        if (m_position < m_text.size() && (m_text[m_position] == 'e' || m_text[m_position] == 'E'))
        {
            ++m_position;
            if (m_position < m_text.size() && (m_text[m_position] == '+' || m_text[m_position] == '-'))
            {
                ++m_position;
            }
            digits = false;
            scanDigits();
            if (!digits)
            {
                fail("Invalid exponent");
            }
        }
        std::istringstream input(m_text.substr(start, m_position - start));
        input.imbue(std::locale::classic());
        double value = 0;
        if (!(input >> value) || !std::isfinite(value))
        {
            fail("Number is out of range");
        }
        m_program.push_back(std::make_unique<PushCommand>(value));
    }

    void function(unsigned depth)
    {
        auto const start = m_position++;
        while (m_position < m_text.size() &&
               (isLetter(m_text[m_position]) || isDigit(m_text[m_position])))
        {
            ++m_position;
        }
        auto const name = m_text.substr(start, m_position - start);
        auto const operation = m_operations.find(name);
        if (!operation || operation->info().binary)
        {
            fail("Unknown function: " + name);
        }
        require('(');
        unsigned count = 0;
        if (peek() != ')')
        {
            do
            {
                if (count != 0)
                {
                    require(',');
                }
                expression(0, depth + 1);
                ++count;
            } while (peek() == ',');
        }
        require(')');
        if (count != operation->info().arity)
        {
            fail("Wrong argument count for " + name);
        }
        m_program.push_back(std::make_unique<OperationCommand>(operation));
    }

    std::string const& m_text;
    IOperationResolver const& m_operations;
    std::size_t m_position = 0;
    Program m_program;
};
}

Program Parser::parse(std::string const& expression) const
{
    if (expression.size() > 65536 || expression.find('\0') != std::string::npos)
    {
        throw std::runtime_error("Invalid expression length or embedded NUL");
    }
    return ParseState(expression, m_operations).parse();
}
