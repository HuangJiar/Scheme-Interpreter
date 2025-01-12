#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include <cassert>
#include <utility> // for std::forward
#include <cstddef> // for size_t

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class SharedPtrController {
    public:
        explicit SharedPtrController() noexcept :shared_ptr_cnt(0), weak_ptr_cnt(0), ptr(nullptr) {}

        explicit SharedPtrController(T* _ptr) noexcept :shared_ptr_cnt(0), weak_ptr_cnt(0), ptr(_ptr) {}

        SharedPtrController(SharedPtrController const&) = delete;

        SharedPtrController& operator = (SharedPtrController const&) = delete;

        ~SharedPtrController() {delete ptr;}

    private:
        friend class SharedPtr<T>;
        friend class WeakPtr<T>;
        size_t shared_ptr_cnt, weak_ptr_cnt;
        T* ptr;
};

template <typename T>
class SharedPtr {
    public:
        explicit SharedPtr() noexcept:ptr(nullptr), c_ptr(nullptr) {}

        explicit SharedPtr(T* _ptr):ptr(_ptr), c_ptr(new SharedPtrController<T>(_ptr)) {++c_ptr->shared_ptr_cnt;}

        SharedPtr(SharedPtrController<T>* cc_ptr): ptr(cc_ptr->ptr), c_ptr(cc_ptr) {assert(c_ptr != nullptr); ++c_ptr->shared_ptr_cnt;} 

        SharedPtr(SharedPtr&&_ptr) noexcept :ptr(_ptr.ptr), c_ptr(_ptr.c_ptr) {
            _ptr.c_ptr = nullptr;
            _ptr.ptr = nullptr;
        }

        SharedPtr(const SharedPtr&_ptr) noexcept :ptr(_ptr.ptr), c_ptr(_ptr.c_ptr) {
            if (c_ptr != nullptr)
                ++c_ptr->shared_ptr_cnt;
        }

        ~SharedPtr() {reset();}

        void reset() {
            if (c_ptr != nullptr) {
                --c_ptr->shared_ptr_cnt;
                if (c_ptr->shared_ptr_cnt == 0) {
                    delete ptr;
                    c_ptr->ptr = nullptr;
                    if (c_ptr->weak_ptr_cnt == 0)
                        delete c_ptr;
                }
                c_ptr = nullptr;
            }
            ptr = nullptr;
        }

        void reset(T* _ptr) {
            reset();
            c_ptr = new SharedPtrController<T>(_ptr);
            ++c_ptr->shared_ptr_cnt;
            ptr = _ptr;
        }

        void reset(SharedPtr<T>&&_ptr) {
            if (this != &_ptr) {
                reset();
                c_ptr = _ptr.c_ptr;
                ptr = _ptr.ptr;
                _ptr.c_ptr = nullptr;
                _ptr.ptr = nullptr;
            }
        }
        
        SharedPtr<T>& operator = (const SharedPtr<T>&_ptr) noexcept {
            if (this != &_ptr) {
                reset();
                c_ptr = _ptr.c_ptr;
                if (c_ptr != nullptr)
                    ++c_ptr->shared_ptr_cnt;
                ptr = _ptr.ptr;
            }
            return *this;
        }

        SharedPtr& operator = (SharedPtr&&_ptr) noexcept {
            if (this != &_ptr) {
                reset();
                c_ptr = _ptr.c_ptr;
                ptr = _ptr.ptr;
                _ptr.c_ptr = nullptr;
                _ptr.ptr = nullptr;
            }
            return *this;
        }

        T& operator * () const noexcept {assert(ptr != nullptr); return *ptr;}
        
        T* operator -> () const noexcept {assert(ptr != nullptr); return ptr;}

        explicit operator bool () const noexcept {return c_ptr != nullptr;}

        T* get() const noexcept {return ptr;}

        size_t use_count() const noexcept {return c_ptr == nullptr ? 0 : c_ptr->shared_ptr_cnt;}
        
    private:
        T *ptr;
        SharedPtrController<T>* c_ptr;
        friend class WeakPtr<T>;
};

template <typename T>
bool operator == (const SharedPtr<T>& __a, nullptr_t) noexcept {return !__a; }
template <typename T>
bool operator != (const SharedPtr<T>& __a, nullptr_t) noexcept {return (bool)__a; }
 
template <typename T, typename ... Args>
std::enable_if_t<!std::is_array<T>::value, SharedPtr<T>>
make_shared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
class WeakPtr {
    public:
        explicit WeakPtr() noexcept :ptr(nullptr), c_ptr(nullptr) {}

        explicit WeakPtr(const SharedPtr<T>&_ptr) noexcept :ptr(_ptr.ptr), c_ptr(_ptr.c_ptr) {
            if (c_ptr != nullptr)
                ++c_ptr->weak_ptr_cnt;
        }

        WeakPtr(WeakPtr&&_ptr) noexcept :ptr(_ptr.ptr), c_ptr(_ptr.c_ptr) {
            _ptr.c_ptr = nullptr;
            _ptr.ptr = nullptr;
        }

        WeakPtr(const WeakPtr&_ptr) noexcept :ptr(_ptr.ptr), c_ptr(_ptr.c_ptr) {
            if (c_ptr != nullptr)
                ++c_ptr->weak_ptr_cnt;
        }

        void reset() noexcept {
            if (c_ptr != nullptr) {
                --c_ptr->weak_ptr_cnt;
                if (c_ptr->shared_ptr_cnt == 0 && c_ptr->weak_ptr_cnt == 0)
                    delete c_ptr;
            }
            ptr = nullptr;
            c_ptr = nullptr;
        }

        ~WeakPtr() {reset();}
        
        WeakPtr<T>& operator = (const WeakPtr<T>&_ptr) noexcept {
            if (this != &_ptr) {
                reset();
                c_ptr = _ptr.c_ptr;
                if (c_ptr != nullptr)
                    ++c_ptr->weak_ptr_cnt;
                ptr = _ptr.ptr;
            }
            return *this;
        }

        WeakPtr<T>& operator = (WeakPtr<T>&&_ptr) noexcept {
            if (this != &_ptr) {
                reset();
                c_ptr = _ptr.c_ptr;
                ptr = _ptr.ptr;
                _ptr.c_ptr = nullptr;
                _ptr.ptr = nullptr;
            }
            return *this;
        }

        WeakPtr<T>& operator = (const SharedPtr<T>&_ptr) noexcept {
            reset();
            c_ptr = _ptr.c_ptr;
            if (c_ptr != nullptr)
                ++c_ptr->weak_ptr_cnt;
            ptr = _ptr.ptr;
            return *this;
        }

        bool expired() const noexcept {return c_ptr == nullptr || c_ptr->shared_ptr_cnt == 0;}

        T& operator * () const noexcept {assert(!expired()); return *ptr;}
        
        T* operator -> () const noexcept {assert(!expired()); return ptr;}

        explicit operator bool () const noexcept {return !expired();}

        size_t use_count() const noexcept {return expired() ? 0 : c_ptr->shared_ptr_cnt;}

        SharedPtr<T> lock() const noexcept {
            if (!expired())
                return SharedPtr<T>(c_ptr);
            else
                return SharedPtr<T>();
        }

        void swap(WeakPtr&_ptr) noexcept {
            std::swap(ptr, _ptr.ptr);
            std::swap(c_ptr, _ptr.c_ptr);
        }

    private:
        T *ptr;
        SharedPtrController<T>* c_ptr;
};

template<typename _Tp>
inline void
    swap(WeakPtr<_Tp>& __a, WeakPtr<_Tp>& __b) noexcept
    { __a.swap(__b); }


#endif