#include "memoryarena.h"



template<typename T> template<class U>
bool ArenaAllocator<T>::operator==(const ArenaAllocator<U>& other) const noexcept{
  return _arena == other._arena;
}

template<typename T> template<typename U>
bool ArenaAllocator<T>::operator!=(const ArenaAllocator<U>& other) const noexcept {
  return _arena != other._arena; 
}

