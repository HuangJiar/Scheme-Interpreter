#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include <cassert>
#include <type_traits> // for std::enable_if_t, std::if_is_array
#include <utility> // for std::forward
#include <cstddef> // for size_t

template <typename T>
class UniquePtr {
    public:
        explicit UniquePtr(T* _ptr = nullptr):ptr(_ptr) {}

        void reset() {delete ptr; ptr = nullptr;}

        T* release() {T* ptr1 = ptr; ptr = nullptr; return ptr1;}

        UniquePtr(UniquePtr&&_ptr):ptr(_ptr.release()) {}

        void reset(T* _ptr) {reset(), ptr = _ptr;}

        void reset(UniquePtr&&_ptr) noexcept {reset(), ptr = _ptr.release();}

        ~UniquePtr() {delete ptr;}

        UniquePtr(const UniquePtr&) = delete;

        UniquePtr& operator = (const UniquePtr&) = delete;

        UniquePtr& operator = (UniquePtr&& _ptr) noexcept {
            if (this != &_ptr)
                reset(), ptr = _ptr.release();
            return *this;
        }

        T& operator * () {return *ptr;}
        
        T* operator -> () {return ptr;}

        explicit operator bool () const {return ptr != nullptr;}

        T* get() const {return ptr;}
        
    private:
        T *ptr;
};
 
template <typename T, typename ... Args>
std::enable_if_t<!std::is_array<T>::value, UniquePtr<T>>
make_unique(Args&&... args) noexcept {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

#endif