#ifndef __EMSCRIPTEN__

#include "hello_imgui/internal/whereami/whereami.h"
#include "hello_imgui/internal/whereami/whereami_cpp.h"

#include "vd_path/vd_system_path.h"
/** @brief Query executable through the common native mechanism, exporting explicit UTF-8. */
std::string wai_getExecutablePath_string()
{
    auto result = vdas::path::executablePath();
    if (!result) throw std::runtime_error("Executable path unavailable");
    return result.value().utf8();
}

std::string wai_getModulePath_string()
{
    int length = wai_getModulePath(NULL, 0, NULL);
    if (length <= 0) throw std::runtime_error("Module path unavailable");
    char* path = (char*)malloc(length + 1);
    if (!path) throw std::bad_alloc();
    if (wai_getModulePath(path, length, NULL) != length)
    {
        free(path);
        throw std::runtime_error("Module path changed during query");
    }
    path[length] = '\0';
    std::string result(path);
    free(path);
    auto validated = vdas::path::parseUtf8(result, "whereami-utf8");
    if (!validated) throw std::runtime_error("Invalid module path encoding");
    return validated.value().utf8();
}

namespace
{
    std::string dirname(const std::string& str)
    {
        size_t found;
        found=str.find_last_of("/\\");
        std::string dirname = str.substr(0,found);
        return dirname;
    }
}

std::string wai_getExecutableFolder_string()
{
    return dirname(wai_getExecutablePath_string());
}

#endif
