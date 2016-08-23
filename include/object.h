/*
    nanogui/object.h -- Object base class with support for reference counting

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef __NGSDL_OBJECT_H__
#define __NGSDL_OBJECT_H__

#include <include/vector2.h>
#include <atomic>

NAMESPACE_BEGIN(nanogui)

/// Reference counted object base class
class NANOGUI_EXPORT Object {
public:
    /// Default constructor
    Object() { }

    /// Copy constructor
    Object(const Object &) : m_refCount(0) {}

    /// Return the current reference count
    int getRefCount() const { return m_refCount; }

    /// Increase the object's reference count by one
    void incRef() const { ++m_refCount; }

    /** \brief Decrease the reference count of
     * the object and possibly deallocate it.
     *
     * The object will automatically be deallocated once
     * the reference count reaches zero.
     */
    void decRef(bool dealloc = true) const
    {
        --m_refCount;
        if (m_refCount == 0 && dealloc)
            delete this;
        else if (m_refCount < 0)
            throw std::runtime_error("Internal error: reference count < 0!");
    }
protected:
    /** \brief Virtual protected deconstructor.
     * (Will only be called by \ref ref)
     */
    virtual ~Object() { }
private:
    mutable std::atomic<int> m_refCount { 0 };
};

/**
 * \brief Reference counting helper
 *
 * The \a ref refeference template is a simple wrapper to store a
 * pointer to an object. It takes care of increasing and decreasing
 * the reference count of the object. When the last reference goes
 * out of scope, the associated object will be deallocated.
 *
 * \ingroup libcore
 */
template <typename T> class ref {
public:
    /// Create a nullptr reference
    ref() : m_ptr(nullptr) { }

    /// Construct a reference from a pointer
    ref(T *ptr) : m_ptr(ptr) {
        if (m_ptr) ((Object *) m_ptr)->incRef();
    }

    /// Copy constructor
    ref(const ref &r) : m_ptr(r.m_ptr) {
        if (m_ptr)
            ((Object *) m_ptr)->incRef();
    }

    /// Move constructor
    ref(ref &&r) : m_ptr(r.m_ptr) {
        r.m_ptr = nullptr; 
    }

    /// Destroy this reference
    ~ref() {
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
    }

    /// Move another reference into the current one
    ref& operator=(ref&& r) {
        if (*this == r)
            return *this;
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
        m_ptr = r.m_ptr;
        r.m_ptr = nullptr;
        return *this;
    }

    /// Overwrite this reference with another reference
    ref& operator=(const ref& r) {
        if (m_ptr == r.m_ptr)
            return *this;
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
        m_ptr = r.m_ptr;
        if (m_ptr)
            ((Object *) m_ptr)->incRef();
        return *this;
    }

    /// Overwrite this reference with a pointer to another object
    ref& operator=(T *ptr) {
        if (m_ptr == ptr)
            return *this;
        if (m_ptr)
            ((Object *) m_ptr)->decRef();
        m_ptr = ptr;
        if (m_ptr)
            ((Object *) m_ptr)->incRef();
        return *this;
    }

    /// Compare this reference with another reference
    bool operator==(const ref &r) const { return m_ptr == r.m_ptr; }

    /// Compare this reference with another reference
    bool operator!=(const ref &r) const { return m_ptr != r.m_ptr; }

    /// Compare this reference with a pointer
    bool operator==(const T* ptr) const { return m_ptr == ptr; }

    /// Compare this reference with a pointer
    bool operator!=(const T* ptr) const { return m_ptr != ptr; }

    /// Access the object referenced by this reference
    T* operator->() { return m_ptr; }

    /// Access the object referenced by this reference
    const T* operator->() const { return m_ptr; }

    /// Return a C++ reference to the referenced object
    T& operator*() { return *m_ptr; }

    /// Return a const C++ reference to the referenced object
    const T& operator*() const { return *m_ptr; }

    /// Return a pointer to the referenced object
    operator T* () { return m_ptr; }

    /// Return a const pointer to the referenced object
    T* get() { return m_ptr; }

    /// Return a pointer to the referenced object
    const T* get() const { return m_ptr; }

    /// Check if the object is defined
    operator bool() const { return m_ptr != nullptr; }
private:
    T *m_ptr;
};

NAMESPACE_END(nanogui)

#endif //__NGSDL_OBJECT_H__
