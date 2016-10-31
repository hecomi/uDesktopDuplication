using System;
using System.Text;
using System.Runtime.InteropServices;

namespace uDesktopDuplication
{

public enum PointerShapeType
{
    MonoChrome = 1,
    Color = 2,
    MaskedColor = 4,
}

public static class Lib
{
    [DllImport("uDesktopDuplication")]
    public static extern int GetMonitorCount();
    [DllImport("uDesktopDuplication")]
    public static extern void SetTimeout(int timeout);
    [DllImport("uDesktopDuplication")]
    public static extern IntPtr GetRenderEventFunc();
    [DllImport("uDesktopDuplication")]
    public static extern void GetName(int id, StringBuilder buf, int len);
    [DllImport("uDesktopDuplication")]
    public static extern int GetWidth(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetHeight(int id);
    [DllImport("uDesktopDuplication")]
    public static extern bool IsPrimary(int id);
    [DllImport("uDesktopDuplication")]
    public static extern bool IsPointerVisible(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetPointerX(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetPointerY(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetPointerShapeWidth(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetPointerShapeHeight(int id);
    [DllImport("uDesktopDuplication")]
    public static extern int GetPointerShapePitch(int id);
    [DllImport("uDesktopDuplication")]
    public static extern PointerShapeType GetPointerShapeType(int id);
    [DllImport("uDesktopDuplication")]
    public static extern void UpdatePointerTexture(int id, System.IntPtr ptr);
    [DllImport("uDesktopDuplication")]
    public static extern int SetTexturePtr(int id, IntPtr ptr);
    [DllImport("uDesktopDuplication")]
    public static extern int GetErrorCode();
    [DllImport("uDesktopDuplication")]
    public static extern void GetErrorMessage(StringBuilder buf, int len);

    public static string GetName(int id)
    {
        var buf = new StringBuilder(32);
        GetName(id, buf, buf.Capacity);
        return buf.ToString();
    }

    public static string GetErrorMessage()
    {
        var buf = new StringBuilder(64);
        GetErrorMessage(buf, buf.Capacity);
        return buf.ToString();
    }
}

}