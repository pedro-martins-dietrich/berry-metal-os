#include "format.hpp"

namespace format
{
    // Temporary string buffer for parsing
    struct Output
    {
        char* buffer;
        size_t capacity;
        size_t length = 0;

        void put(char value)
        {
            if(capacity > 0 && length < capacity - 1)
                buffer[length] = value;
            ++length;
        }

        void string(const char* value)
        {
            if(!value)
                value = "(null)";
            while(*value)
                put(*value++);
        }

        void number(unsigned int value, unsigned int base, bool uppercase)
        {
            char digits[sizeof(unsigned int) * 8];
            size_t count = 0;
            const char* alphabet = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
            do
            {
                digits[count++] = alphabet[value % base];
                value /= base;
            } while(value != 0);
            while(count > 0)
                put(digits[--count]);
        }
    };
}

size_t format::snprintf(char* out_buffer, size_t capacity, const char* pattern, ...)
{
    va_list args;
    va_start(args, pattern);
    const size_t length = vsnprintf(out_buffer, capacity, pattern, args);
    va_end(args);

    return length;
}

size_t format::vsnprintf(char* out_buffer, size_t capacity, const char* pattern, va_list arguments)
{
    if(!out_buffer || !pattern) return 0;

    Output output{out_buffer, capacity};
    va_list args;
    va_copy(args, arguments);

    while(*pattern)
    {
        if(*pattern != '%')
        {
            output.put(*pattern++);
            continue;
        }
        pattern++;

        switch(*pattern)
        {
            case '\0':
            case '%':
                output.put('%');
                break;
            case 'c':
                output.put(static_cast<char>(va_arg(args, int)));
                break;
            case 's':
                output.string(va_arg(args, const char*));
                break;
            case 'd':
            case 'i':
            {
                const int value = va_arg(args, int);
                unsigned int magnitude = static_cast<unsigned int>(value);
                if(value < 0)
                {
                    output.put('-');
                    magnitude = 0U - magnitude;
                }
                output.number(magnitude, 10U, false);
                break;
            }
            case 'u':
                output.number(va_arg(args, unsigned int), 10U, false);
                break;
            case 'x':
                output.put('0');
                output.put('x');
                output.number(va_arg(args, unsigned int), 16U, false);
                break;
            case 'X':
                output.put('0');
                output.put('x');
                output.number(va_arg(args, unsigned int), 16U, true);
                break;
            default:
                output.put('%');
                output.put(*pattern);
                break;
        }
        if(*pattern)
            pattern++;
    }
    va_end(args);

    if(capacity > 0)
        out_buffer[output.length < capacity ? output.length : capacity - 1] = '\0';
    return output.length;
}
