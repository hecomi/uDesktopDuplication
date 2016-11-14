using UnityEngine;
using System.Collections;
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

    public static void WaitThenDo(System.Action func, float sec)
    {
        Manager.instance.StartCoroutine(_WaitThenDo(func, sec));
    }

    public static IEnumerator _WaitThenDo(System.Action func, float sec)
    {
        yield return new WaitForSeconds(sec);
        func();
    }
}

}