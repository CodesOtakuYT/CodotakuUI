#pragma once

#include <streambuf>
#include <string>
#include <string_view>
#include <iostream>
#include <stdexcept>
#include <format>
#include <source_location>

#include "SDL3/SDL_log.h"
#include "SDL3/SDL_init.h"

class SDLLogStreambuf : public std::streambuf {
public:
    explicit SDLLogStreambuf(const SDL_LogPriority priority) : priority_{priority} {
    }

protected:
    int_type overflow(int_type c) override {
        if (c == traits_type::eof()) return traits_type::eof();
        if (c == '\n') flushBuffer();
        else buffer_ += traits_type::to_char_type(c);
        return c;
    }

    int sync() override {
        flushBuffer();
        return 0;
    }

    std::streamsize xsputn(const char *ptr, std::streamsize count) override {
        std::string_view view(ptr, count);
        if (const auto pos = view.find('\n'); pos == std::string_view::npos) {
            buffer_.append(view);
        } else {
            buffer_.append(view, 0, pos);
            flushBuffer();
            if (pos + 1 < count) xsputn(ptr + pos + 1, count - pos - 1);
        }
        return count;
    }

private:
    void flushBuffer() {
        if (buffer_.empty()) return;
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, priority_, "%s", buffer_.c_str());
        buffer_.clear();
    }

    SDL_LogPriority priority_;
    std::string buffer_;
};

class SDLLogRedirector {
public:
    SDLLogRedirector()
        : cout_buffer_(SDL_LOG_PRIORITY_INFO),
          cerr_buffer_(SDL_LOG_PRIORITY_ERROR) {
        old_cout_ = std::cout.rdbuf(&cout_buffer_);
        old_cerr_ = std::cerr.rdbuf(&cerr_buffer_);
    }

    ~SDLLogRedirector() {
        std::cout.rdbuf(old_cout_);
        std::cerr.rdbuf(old_cerr_);
    }

    SDLLogRedirector(const SDLLogRedirector &) = delete;

    SDLLogRedirector &operator=(const SDLLogRedirector &) = delete;

private:
    SDLLogStreambuf cout_buffer_;
    SDLLogStreambuf cerr_buffer_;
    std::streambuf *old_cout_;
    std::streambuf *old_cerr_;
};

class SDLException : public std::runtime_error {
public:
    explicit SDLException(const std::source_location &loc = std::source_location::current())
        : std::runtime_error{
            std::format("{}:{}:{} ({}) : {}", loc.file_name(), loc.line(), loc.column(), loc.function_name(),
                        SDL_GetError())
        } {
    }
};

inline auto chk(bool result) -> void {
    if (!result)
        throw SDLException{};
}

template<typename T>
auto chk(T *result) -> T * {
    if (!result)
        throw SDLException{};
    return result;
}

template<typename T>
auto chk(T result) -> T {
    if (!result)
        throw SDLException{};
    return result;
}
