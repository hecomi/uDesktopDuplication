using UnityEngine;
using System;
using System.Text;
using System.Runtime.InteropServices;

#pragma warning disable 114, 465

namespace uDesktopDuplication
{

public enum Message
{
    None = -1,
    Reinitialized = 0,
    TextureSizeChanged = 1,
}

public enum CursorShapeType
{
    Unspecified = 0,
    MonoChrome = 1,
    Color = 2,
    MaskedColor = 4,
}

public enum MonitorRotation
{
    Unspecified = 0,
    Identity = 1,
    Rotate90 = 2,
    Rotate180 = 3,
    Rotate270 = 4
}

public enum DuplicatorState
{
    NotSet = -1,
    Ready = 0,
    Running = 1,
    InvalidArg = 2,
    AccessDenied = 3,
    Unsupported = 4,
    CurrentlyNotAvailable = 5,
    SessionDisconnected = 6,
    AccessLost = 7,
    TextureSizeInconsistent = 8,
    Unknown = 999,
}

public enum DebugMode
{
    None = 0,
    File = 1,
    UnityLog = 2, /* currently has bug when app exits. */
}

[StructLayout(LayoutKind.Sequential)]
public struct RECT
{
    [MarshalAs(UnmanagedType.I4)]
    public int left;
    [MarshalAs(UnmanagedType.I4)]
    public int top;
    [MarshalAs(UnmanagedType.I4)]
    public int right;
    [MarshalAs(UnmanagedType.I4)]
    public int bottom;
}

[StructLayout(LayoutKind.Sequential)]
public struct DXGI_OUTDUPL_MOVE_RECT
{
    public RECT source;
    public RECT destination;
}

public static class Lib
{
    const string dllName = "uDesktopDuplication";

    public delegate void MessageHandler(Message message);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void DebugLogDelegate(string str);

    [DllImport(dllName)]
    public static extern bool IsInitialized();
    [DllImport(dllName)]
    public static extern void Initialize();
    [DllImport(dllName)]
    public static extern void Finalize();
    [DllImport(dllName)]
    public static extern void Reinitialize();
    [DllImport(dllName)]
    public static extern void Update();
    [DllImport(dllName)]
    public static extern Message PopMessage();
    [DllImport(dllName)]
    public static extern void EnableDebug();
    [DllImport(dllName)]
    public static extern void DisableDebug();
    [DllImport(dllName)]
    public static extern void SetDebugMode(DebugMode mode);
    [DllImport(dllName)]
    public static extern void SetLogFunc(DebugLogDelegate func);
    [DllImport(dllName)]
    public static extern void SetErrorFunc(DebugLogDelegate func);
    [DllImport(dllName)]
    public static extern int GetMonitorCount();
    [DllImport(dllName)]
    public static extern bool HasMonitorCountChanged();
    [DllImport(dllName)]
    public static extern int GetCursorMonitorId();
    [DllImport(dllName)]
    public static extern int GetTotalWidth();
    [DllImport(dllName)]
    public static extern int GetTotalHeight();
    [DllImport(dllName)]
    public static extern IntPtr GetRenderEventFunc();
    [DllImport(dllName)]
    public static extern int GetId(int id);
    [DllImport(dllName)]
    public static extern DuplicatorState GetState(int id);
    [DllImport(dllName)]
    public static extern void GetName(int id, StringBuilder buf, int len);
    [DllImport(dllName)]
    public static extern int GetLeft(int id);
    [DllImport(dllName)]
    public static extern int GetRight(int id);
    [DllImport(dllName)]
    public static extern int GetTop(int id);
    [DllImport(dllName)]
    public static extern int GetBottom(int id);
    [DllImport(dllName)]
    public static extern int GetWidth(int id);
    [DllImport(dllName)]
    public static extern int GetHeight(int id);
    [DllImport(dllName)]
    public static extern int GetDpiX(int id);
    [DllImport(dllName)]
    public static extern int GetDpiY(int id);
    [DllImport(dllName)]
    public static extern bool IsHDR(int id);
    [DllImport(dllName)]
    public static extern MonitorRotation GetRotation(int id);
    [DllImport(dllName)]
    public static extern bool IsPrimary(int id);
    [DllImport(dllName)]
    public static extern bool IsCursorVisible();
    [DllImport(dllName)]
    public static extern int GetCursorX();
    [DllImport(dllName)]
    public static extern int GetCursorY();
    [DllImport(dllName)]
    public static extern int GetCursorShapeWidth();
    [DllImport(dllName)]
    public static extern int GetCursorShapeHeight();
    [DllImport(dllName)]
    public static extern int GetCursorShapePitch();
    [DllImport(dllName)]
    public static extern CursorShapeType GetCursorShapeType();
    [DllImport(dllName)]
    public static extern void GetCursorTexture(IntPtr ptr);
    [DllImport(dllName)]
    public static extern int GetCursorHotSpotX();
    [DllImport(dllName)]
    public static extern int GetCursorHotSpotY();
    [DllImport(dllName)]
    public static extern int SetTexturePtr(int id, IntPtr ptr);
    [DllImport(dllName)]
    public static extern IntPtr GetSharedTextureHandle(int id);
    [DllImport(dllName)]
    public static extern int GetMoveRectCount(int id);
    [DllImport(dllName, EntryPoint = "GetMoveRects")]
    private static extern IntPtr GetMoveRects_Internal(int id);
    [DllImport(dllName)]
    public static extern int GetDirtyRectCount(int id);
    [DllImport(dllName, EntryPoint = "GetDirtyRects")]
    private static extern IntPtr GetDirtyRects_Internal(int id);
    [DllImport(dllName, EntryPoint = "GetPixels")]
    private static extern bool GetPixels_Internal(int id, IntPtr ptr, int x, int y, int width, int height);
    [DllImport(dllName)]
    public static extern IntPtr GetBuffer(int id);
    [DllImport(dllName)]
    public static extern bool HasBeenUpdated(int id);
    [DllImport(dllName)]
    public static extern bool UseGetPixels(int id, bool use);
    [DllImport(dllName)]
    public static extern void SetFrameRate(uint frameRate);

    public static string GetName(int id)
    {
        var buf = new StringBuilder(32);
        GetName(id, buf, buf.Capacity);
        return buf.ToString();
    }

    public static DXGI_OUTDUPL_MOVE_RECT[] GetMoveRects(int id)
    {
        var count = GetMoveRectCount(id);
        var rects = new DXGI_OUTDUPL_MOVE_RECT[count];
        var ptr = GetMoveRects_Internal(id);
        var size = Marshal.SizeOf(typeof(DXGI_OUTDUPL_MOVE_RECT));
        for (int i = 0; i < count; ++i) {
            var data = new IntPtr(ptr.ToInt64() + size * i);
            rects[i] = (DXGI_OUTDUPL_MOVE_RECT)Marshal.PtrToStructure(data, typeof(DXGI_OUTDUPL_MOVE_RECT));
        }
        return rects;
    }

    public static RECT[] GetDirtyRects(int id)
    {
        var count = GetDirtyRectCount(id);
        var rects = new RECT[count];
        var ptr = GetDirtyRects_Internal(id);
        var size = Marshal.SizeOf(typeof(RECT));
        for (int i = 0; i < count; ++i) {
            var data = new IntPtr(ptr.ToInt64() + size * i);
            rects[i] = (RECT)Marshal.PtrToStructure(data, typeof(RECT));
        }
        return rects;
    }

    public static Color32[] GetPixels(int id, int x, int y, int width, int height)
    {
        var color = new Color32[width * height];       
        GetPixels(id, color, x, y, width, height);
        return color;
    }

    public static bool GetPixels(int id, Color32[] colors, int x, int y, int width, int height)
    {
        if (colors.Length < width * height) {
            Debug.LogErrorFormat("colors is small.", id, x, y, width, height);
            return false;
        }
		var handle = GCHandle.Alloc(colors, GCHandleType.Pinned);
		var ptr = handle.AddrOfPinnedObject();
        if (!GetPixels_Internal(id, ptr, x, y, width, height)) {
            Debug.LogErrorFormat("GetPixels({0}, {1}, {2}, {3}, {4}) failed.", id, x, y, width, height);
            return false;
        }
        handle.Free();
        return true;
    }

    public static Color32 GetPixel(int id, int x, int y)
    {
        return GetPixels(id, x, y, 1, 1)[0];
    }
}

}