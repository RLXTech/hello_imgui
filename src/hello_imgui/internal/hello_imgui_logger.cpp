#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/imguial_term.h"
#include <mutex>
namespace HelloImGui
{

namespace InternalLogBuffer
{
    static std::mutex mutex_;
    static constexpr size_t gMaxBufferSize = 600000;
    char gLogBuffer_[gMaxBufferSize];
    ImGuiAl::Log gLog(gLogBuffer_, gMaxBufferSize);

}  // namespace InternalLogBuffer

void Log(LogLevel level, char const* const format, ...)
{
    std::scoped_lock lock(InternalLogBuffer::mutex_);
    va_list args;
    va_start(args, format);

    if (level == LogLevel::Debug)
        InternalLogBuffer::gLog.debug(format, args);
    else if (level == LogLevel::Info)
        InternalLogBuffer::gLog.info(format, args);
    else if (level == LogLevel::Warning)
        InternalLogBuffer::gLog.warning(format, args);
    else if (level == LogLevel::Error)
        InternalLogBuffer::gLog.error(format, args);
    else
        throw std::runtime_error("Log: bad LogLevel !");

    va_end(args);
}

void LogClear() { InternalLogBuffer::gLog.clear(); }

void LogGui(ImVec2 size)
{
    std::scoped_lock lock(InternalLogBuffer::mutex_);
    InternalLogBuffer::gLog.draw(size);
}

}  // namespace HelloImGui
