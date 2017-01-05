#pragma once

#include <functional>
#include <memory>
#include <wrl/client.h>


// Unity interface getter
struct IUnityInterfaces;
IUnityInterfaces* GetUnity();

// ID3D11Device (in Unity) getter
struct ID3D11Device;
Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();

// Manager getter
class MonitorManager;
const std::unique_ptr<MonitorManager>& GetMonitorManager();

// Get adapter LUID to check the adapter of the monitor is same as Unity one.
LUID GetUnityAdapterLuid();


// Message is pooled and fetch from Unity.
enum class Message
{
    None = -1,
    Reinitialized = 0,
    TextureSizeChanged = 1,
};

void SendMessageToUnity(Message message);


// Buffer
template <class T>
class Buffer
{
public:
    Buffer() {}
    ~Buffer() {}

    void ExpandIfNeeded(UINT size)
    {
        if (size > size_)
        {
            size_ = size;
            value_ = std::make_unique<T[]>(size);
        }
    }

    void Reset()
    {
        value_.reset();
        size_ = 0;
    }

    UINT Size() const
    {
        return size_;
    }

    T* Get() const
    {
        return value_.get();
    }

    T* Get(UINT offset) const
    {
        return (value_.get() + offset);
    }

    template <class U>
    U* As() const
    {
        return reinterpret_cast<U*>(Get());
    }

    template <class U>
    U* As(UINT offset) const
    {
        return reinterpret_cast<U*>(Get(offset));
    }

    operator bool() const
    {
        return value_ != nullptr;
    }

    const T operator [](UINT index) const
    {
        if (index >= size_)
        {
            Debug::Error("Array index out of range: ", index, size_);
            return T(0);
        }
        return value_[index];
    }

    T& operator [](UINT index)
    {
        if (index >= size_)
        {
            Debug::Error("Array index out of range: ", index, size_);
            return value_[0];
        }
        return value_[index];
    }

private:
    std::unique_ptr<T[]> value_;
    UINT size_ = 0;
};