#include "PluginLoader.h"
#include "PluginApi.h"
#include <algorithm>
#include <cmath>
#include <ostream>
#include <stdexcept>
#include <utility>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace
{
class SharedLibrary final
{
public:
    explicit SharedLibrary(std::filesystem::path const& path)
    {
#ifdef _WIN32
        m_handle = LoadLibraryW(path.c_str());
        if (!m_handle)
        {
            throw std::runtime_error("LoadLibrary failed, code " + std::to_string(GetLastError()));
        }
#else
        m_handle = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!m_handle)
        {
            throw std::runtime_error(dlerror());
        }
#endif
    }

    ~SharedLibrary() noexcept
    {
#ifdef _WIN32
        FreeLibrary(m_handle);
#else
        dlclose(m_handle);
#endif
    }

    SharedLibrary(SharedLibrary const&) = delete;
    SharedLibrary& operator=(SharedLibrary const&) = delete;
    SharedLibrary(SharedLibrary&&) = delete;
    SharedLibrary& operator=(SharedLibrary&&) = delete;

    GetPluginInfo entry() const
    {
#ifdef _WIN32
        auto const symbol = GetProcAddress(m_handle, "getPluginInfo");
#else
        auto const symbol = dlsym(m_handle, "getPluginInfo");
#endif
        if (!symbol)
        {
            throw std::runtime_error("Missing export getPluginInfo");
        }
        return reinterpret_cast<GetPluginInfo>(symbol);
    }
private:
#ifdef _WIN32
    HMODULE m_handle = nullptr;
#else
    void* m_handle = nullptr;
#endif
};

bool validName(std::string const& name)
{
    auto letter = [](char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    };
    return !name.empty() && letter(name.front()) &&
        std::all_of(name.begin(), name.end(), [&](char c)
        {
            return letter(c) || (c >= '0' && c <= '9');
        });
}

// Adapter: переводит контракт DLL в интерфейс операций калькулятора.
class PluginOperation final : public IOperation
{
public:
    explicit PluginOperation(std::filesystem::path const& path) : m_library(path)
    {
        auto const* descriptor = m_library.entry()();
        if (!descriptor || descriptor->version != PluginApiVersion ||
            !descriptor->name || !descriptor->evaluate)
        {
            throw std::runtime_error("Invalid plugin descriptor or API version");
        }
        m_info = {descriptor->name, descriptor->arity, descriptor->kind == BinaryOperator,
                  descriptor->precedence, descriptor->rightAssociative != 0};
        if (descriptor->kind > BinaryOperator || m_info.arity > 16 ||
            descriptor->rightAssociative > 1)
        {
            throw std::runtime_error("Invalid plugin metadata");
        }
        if (m_info.binary)
        {
            std::string const symbols = "+-*/^%&|~!<>";
            if (m_info.name.size() != 1 || symbols.find(m_info.name[0]) == std::string::npos ||
                m_info.arity != 2 || m_info.precedence == 0 || m_info.precedence > 100)
            {
                throw std::runtime_error("Invalid binary operator metadata");
            }
        }
        else if (!validName(m_info.name))
        {
            throw std::runtime_error("Invalid function name");
        }
        m_evaluate = descriptor->evaluate;
    }

    OperationInfo const& info() const override { return m_info; }

    double calculate(std::vector<double> const& arguments) const override
    {
        if (arguments.size() != m_info.arity)
        {
            throw std::runtime_error("Wrong plugin argument count");
        }
        double result = 0;
        char error[512] = {};
        auto const status = m_evaluate(arguments.data(), static_cast<unsigned>(arguments.size()),
                                       &result, error, sizeof(error));
        error[sizeof(error) - 1] = '\0';
        if (status != 0)
        {
            throw std::runtime_error(m_info.name + ": " + (error[0] ? error : "Plugin failed"));
        }
        if (!std::isfinite(result))
        {
            throw std::runtime_error(m_info.name + ": non-finite result");
        }
        return result;
    }
private:
    // Дескриптор библиотеки живёт дольше всех данных и адресов её функций.
    SharedLibrary m_library;
    OperationInfo m_info;
    EvaluateFunction m_evaluate = nullptr;
};
}

void PluginLoader::load(std::filesystem::path const& directory, OperationRegistry& operations,
                        std::ostream& diagnostics) const
{
    std::error_code error;
    std::filesystem::directory_iterator it(directory, error);
    if (error)
    {
        diagnostics << "Warning: cannot open plugin directory: " << error.message() << '\n';
        return;
    }
    std::vector<std::filesystem::path> files;
    for (; it != std::filesystem::directory_iterator(); it.increment(error))
    {
        if (error)
        {
            break;
        }
        auto extension = it->path().extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c)
        {
            return static_cast<char>(c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c);
        });
#ifdef _WIN32
        bool const candidate = extension == ".dll";
#elif defined(__APPLE__)
        bool const candidate = extension == ".dylib" || extension == ".so";
#else
        bool const candidate = extension == ".so";
#endif
        if (candidate)
        {
            files.push_back(it->path());
        }
    }
    if (error)
    {
        diagnostics << "Warning: plugin directory scan failed: " << error.message() << '\n';
    }
    std::sort(files.begin(), files.end());
    unsigned loaded = 0;
    for (auto const& file : files)
    {
        try
        {
            operations.add(std::make_shared<PluginOperation>(std::filesystem::absolute(file)));
            ++loaded;
        }
        catch (std::bad_alloc const&)
        {
            throw;
        }
        catch (std::exception const& exception)
        {
            diagnostics << "Warning: " << file.filename().string() << ": " << exception.what() << '\n';
        }
        catch (...)
        {
            diagnostics << "Warning: " << file.filename().string() << ": unknown plugin error\n";
        }
    }
    if (loaded == 0)
    {
        diagnostics << "Warning: no plugins loaded; basic arithmetic is available\n";
    }
}
