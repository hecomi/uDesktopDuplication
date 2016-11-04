using System;
using System.Text;
using System.Runtime.InteropServices;

namespace uDesktopDuplication
{

public enum Message
{
    None = -1,
    Reinitialized = 0,
}

public enum CursorShapeType
{
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

public static class Lib
{
    public delegate void MessageHandler(Message message);

    [DllImport("uDesktopDuplication")]
    public static extern void InitializeUDD();
    [DllImport("uDesktopDuplication")]
    public static extern void FinalizeUDD();
    [DllImport("uDesktopDuplication")]
    public static extern void Update();
    [DllImport("uDesktopDuplication")]
    public static extern Message PopMessage();
    [DllImport("uDesktopDuplication")]
    public static extern int GetMonitorCount();
    [DllImport("uDesktopDuplication")]
    public static extern int GetTotalWidth();
    [DllImport("uDesktopDuplication")]
    public static extern int GetTotalHeight();
    [DllImport("uDesktopDuplication")]
    public static extern void SetTimeout(int timeout);
    [DllImport("uDesktopDuplication")]
    public static extern IntPtr GetRenderEventFunc();
    [DllImport("uDesktopDuplication")]
    public static extern void GetName(int id, StringBuilder buf, int len);
    [DllImport("uDesktopDuplication")]
    public static extern int GetLeft(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetRight(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetTop(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetBottom(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetWidth(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetHeight(int id);
    [DllImport("uDesktopDuplication")]
    public static extern MonitorRotation GetRotation(int id);
    [DllImport("uDesktopDuplication")]
    public static extern bool IsPrimary(int id);
    [DllImport("uDesktopDuplication")]
    public static extern bool IsCursorVisible(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetCursorX(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetCursorY(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetCursorShapeWidth(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetCursorShapeHeight(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetCursorShapePitch(int id);
    [DllImport("uDesktopDuplication")]
    public static extern CursorShapeType GetCursorShapeType(int id);
    [DllImport("uDesktopDuplication")]
    public static extern void UpdateCursorTexture(int id, System.IntPtr ptr);
    [DllImport("uDesktopDuplication")]
    public static extern int SetTexturePtr(int id, IntPtr ptr);

    public static string GetName(int id)
    {
        var buf = new StringBuilder(32);
        GetName(id, buf, buf.Capacity);
        return buf.ToString();
    }
}

}