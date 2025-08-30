#ifndef PTR_HANDLE_WRAPPER_H
#define PTR_HANDLE_WRAPPER_H

#include <type_traits>

template <typename T>
class PtrHandleWrapper {
  static_assert(std::is_pointer<T>::value,
                "PtrHandleWrapper can only be used with ptr");

 protected:
  T handle_ = nullptr;

 public:
  T getHandle() { return handle_; }
  bool isHandleValid() const { return handle_ != nullptr; }
};

#endif /* PTR_HANDLE_WRAPPER_H */

