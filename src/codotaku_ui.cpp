#include <new>
#include <cstdlib>
#include "SDL3/SDL_stdinc.h"

// No inline keyword here. This compiles into exactly one object file.

void *operator new(std::size_t size) {
    if (size == 0) size = 1;
    void *ptr = SDL_malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void *operator new[](std::size_t size) {
    if (size == 0) size = 1;
    void *ptr = SDL_malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void *operator new(std::size_t size, std::align_val_t al) {
    if (size == 0) size = 1;
    void *ptr = SDL_aligned_alloc(static_cast<size_t>(al), size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void *operator new[](std::size_t size, std::align_val_t al) {
    if (size == 0) size = 1;
    void *ptr = SDL_aligned_alloc(static_cast<size_t>(al), size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void *operator new(std::size_t size, const std::nothrow_t &) noexcept {
    if (size == 0) size = 1;
    return SDL_malloc(size);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept {
    if (size == 0) size = 1;
    return SDL_malloc(size);
}

void operator delete(void *ptr) noexcept { SDL_free(ptr); }
void operator delete[](void *ptr) noexcept { SDL_free(ptr); }
void operator delete(void *ptr, std::size_t) noexcept { SDL_free(ptr); }
void operator delete[](void *ptr, std::size_t) noexcept { SDL_free(ptr); }
void operator delete(void *ptr, std::align_val_t) noexcept { SDL_aligned_free(ptr); }
void operator delete[](void *ptr, std::align_val_t) noexcept { SDL_aligned_free(ptr); }
void operator delete(void *ptr, std::size_t, std::align_val_t) noexcept { SDL_aligned_free(ptr); }
void operator delete[](void *ptr, std::size_t, std::align_val_t) noexcept { SDL_aligned_free(ptr); }
