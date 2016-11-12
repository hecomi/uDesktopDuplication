using UnityEngine;
using System.Runtime.InteropServices;

namespace uDesktopDuplication
{

public struct MousePoint
{
    public int x;
    public int y;
}

public static class Utility
{
    [DllImport("user32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool GetCursorPos(out MousePoint point);

    public static MousePoint GetCursorPos()
    {
        MousePoint p;
        if (!GetCursorPos(out p)) {
            p.x = -1;
            p.y = -1;
        }
        return p;
    }
}

}