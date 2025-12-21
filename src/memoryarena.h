#pragma once
#include <cstddef>
#include <cstdlib>
#include <new>
#include <memory>

class MemoryArena final {
public:
  explicit MemoryArena(size_t bytes) :
    _buff(static_cast<char*>(::operator new(bytes))),
    _capacity(bytes),
    _offset(0) {}

  ~MemoryArena(){
    ::operator delete(_buff);
  }

  template<typename T>
  T* allocate_array(size_t count) {
    return static_cast<T*>(allocate_raw(sizeof(T) * count, alignof(T)));
  }

  template<typename T, typename... Args>
  T* allocate(Args&&... args) {
    void* ptr = allocate_raw(sizeof(T), alignof(T));
    return ::new (ptr) T(std::forward<Args>(args)...);
  }

private:
  char* _buff;
  size_t _capacity;
  size_t _offset;

  void* allocate_raw(size_t size, size_t alignment) {
    size_t space = _capacity - _offset;
    void* ptr = _buff + _offset;
    if (!std::align(alignment, size, ptr, space))
      throw std::bad_alloc{};
    _offset = static_cast<char*>(ptr) - _buff + size;
    return ptr;
  }
};

// https://learn.microsoft.com/en-us/cpp/standard-library/allocators?view=msvc-170

template<typename T>
class ArenaAllocator {
public:
  explicit ArenaAllocator(MemoryArena& arena) noexcept
  : _arena(&arena) {}

  template<typename U> ArenaAllocator(const ArenaAllocator<U>& other) noexcept
  : _arena(other.arena_) {}

  T* allocate(std::size_t n) {
    return _arena->allocate_array<T>(n);
  }

  template<class U> bool operator==(const ArenaAllocator<U>& other) const noexcept;
  template<typename U> bool operator!=(const ArenaAllocator<U>& other) const noexcept;

  void deallocate(T*, std::size_t) noexcept { } 
  using value_type = T;
private:
  MemoryArena* _arena;
};
