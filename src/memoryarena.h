#pragma once
#include <cstring>
#include <vector>
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

  void* allocate_bytes(size_t bytes, size_t alignment) {
    size_t current = reinterpret_cast<size_t>(_buff + _offset);
    size_t aligned = (current + alignment - 1) & ~(alignment - 1);
    size_t newOffset = aligned - reinterpret_cast<size_t>(_buff) + bytes;

    if (newOffset > _capacity)
      throw std::bad_alloc();

    _offset = newOffset;
    return reinterpret_cast<void*>(aligned);
  }

  template<typename T, typename... Args>
  T* allocate(Args&&... args) {
    void* mem = allocate_bytes(sizeof(T), alignof(T));
    return ::new (mem) T(std::forward<Args>(args)...);
  }
private:
  char* _buff;
  size_t _capacity;
  size_t _offset;
};

template<typename T>
struct Slice {
  T* data = nullptr;
  uint32_t size = 0;

  T& at(uint32_t i) const {
    return data[i];
  }

  T* begin() const { return data; }
  T* end()   const { return data + size; }

  bool empty() const { return size == 0; }
};
template<typename T>
Slice<T> makeSlice(MemoryArena& arena, const std::vector<T>& vec) {
  if (vec.empty())
    return { nullptr, 0 };

  T* data = static_cast<T*>(
    arena.allocate_bytes(sizeof(T) * vec.size(), alignof(T))
  );

  for (size_t i = 0; i < vec.size(); ++i) {
    ::new (&data[i]) T(vec[i]); // copy-construct
  }

  return { data, vec.size() };
}
