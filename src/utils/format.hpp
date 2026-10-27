#pragma once

#include <cstdarg>
#include <cstddef>

// String formatting operations
namespace format
{
    // Formats the `pattern` string and outputs it into the `outbuffer`
    size_t snprintf(char* out_buffer, size_t capacity, const char* pattern, ...);
    // Formats the string inside the buffer using a `va_list`
    size_t vsnprintf(char* out_buffer, size_t capacity, const char* pattern, va_list arguments);
}
