using UnityEngine;
using System;
using System.Runtime.InteropServices;

public class BufferExample : MonoBehaviour
{
    [SerializeField]
    uDesktopDuplication.Texture uddTexture;

    Texture2D texture_;
    Color32[] pixels_;
    GCHandle handle_;
    IntPtr ptr_ = IntPtr.Zero;

    [DllImport("msvcrt.dll", CallingConvention = CallingConvention.Cdecl, SetLastError = false)]
    public static extern IntPtr memcpy(IntPtr dest, IntPtr src, int count);

    void Start()
    {
        if (!uddTexture) return;

        uddTexture.monitor.useGetPixels = true;
        UpdateTexture();
    }

    void OnDestroy()
    {
        if (ptr_ != IntPtr.Zero) {
            handle_.Free();
        }
    }

    void Update()
    {
        if (!uddTexture) return;

        if (uddTexture.monitor.width != texture_.width || 
            uddTexture.monitor.height != texture_.height) {
            UpdateTexture();
        }

        CopyTexture();
    }

    void UpdateTexture()
    {
        var width = uddTexture.monitor.width;
        var height = uddTexture.monitor.height;

        // TextureFormat.BGRA32 should be set but it causes an error now.
        texture_ = new Texture2D(width, height, TextureFormat.RGBA32, false);
        texture_.filterMode = FilterMode.Bilinear;
        pixels_ = texture_.GetPixels32();
        handle_ = GCHandle.Alloc(pixels_, GCHandleType.Pinned);
        ptr_ = handle_.AddrOfPinnedObject();

        GetComponent<Renderer>().material.mainTexture = texture_;
    }

    void CopyTexture()
    {
        var buffer = uddTexture.monitor.buffer;
        if (buffer == IntPtr.Zero) return;

        var width = uddTexture.monitor.width;
        var height = uddTexture.monitor.height;
        memcpy(ptr_, buffer, width * height * sizeof(Byte) * 4);

        texture_.SetPixels32(pixels_);
        texture_.Apply();
    }
}
