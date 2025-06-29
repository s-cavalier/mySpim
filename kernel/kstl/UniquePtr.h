#ifndef M__UNIQUE_PTR_H__
#define M__UNIQUE_PTR_H__
#include "Error.h"

namespace ministl {

    template<typename T>
    class unique_ptr {
        T* ptr;
    public:
        explicit unique_ptr(T* p = nullptr) : ptr(p) {}
        ~unique_ptr() { delete ptr; }

        unique_ptr(unique_ptr&& other) : ptr(other.ptr) { other.ptr = nullptr; }
        unique_ptr& operator=(unique_ptr&& other) {
            if (this != &other) {
                delete ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        inline T* get() const { return ptr; }
        inline T* operator->() const { return ptr; }
        inline T& operator*() const { return *ptr; }

        inline T* release() { T* ret = ptr; ptr = nullptr; return ret; }

        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;
    };

    template<typename T> struct remove_reference { using type = T; };
    template<typename T> struct remove_reference<T&> { using type = T; };
    template<typename T> struct remove_reference<T&&> { using type = T; };

    template<typename T>
    constexpr T&& forward(typename remove_reference<T>::type& t) {
        return static_cast<T&&>(t);
    }

    template<typename T>
    constexpr T&& forward(typename remove_reference<T>::type&& t) {
        return static_cast<T&&>(t);
    }

    template<typename T, typename... Args>
    unique_ptr<T> make_unique(Args&&... args) {
        return unique_ptr<T>(new T(forward<Args>(args)...));
    }

    template<typename T>
    constexpr typename remove_reference<T>::type&& move(T&& t) {
        return static_cast<typename remove_reference<T>::type&&>(t);
    }

    template<typename T>
    class unique_ptr<T[]> {
        T* ptr;
    public:
        explicit unique_ptr(T* p = nullptr) : ptr(p) {}
        ~unique_ptr() { delete[] ptr; }

        unique_ptr(unique_ptr&& other) : ptr(other.ptr) { other.ptr = nullptr; }
        unique_ptr& operator=(unique_ptr&& other) {
            if (this != &other) {
                delete[] ptr;
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        T* get() const { return ptr; }
        T& operator[](size_t i) const { return ptr[i]; }

        unique_ptr(const unique_ptr&) = delete;
        unique_ptr& operator=(const unique_ptr&) = delete;
    };

    template<typename T>
    ministl::unique_ptr<T[]> make_unique(size_t n) {
        return ministl::unique_ptr<T[]>(new T[n]());
    }




}




#endif