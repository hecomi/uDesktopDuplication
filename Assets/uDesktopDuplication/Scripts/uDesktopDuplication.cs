using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

public class uDesktopDuplication : MonoBehaviour
{
    [DllImport("uDesktopDuplication")]
    private static extern void Initialize();
    [DllImport("uDesktopDuplication")]
    private static extern int GetWidth();
    [DllImport("uDesktopDuplication")]
    private static extern int GetHeight();
    [DllImport("uDesktopDuplication")]
    private static extern bool IsPointerVisible();
    [DllImport("uDesktopDuplication")]
    private static extern int GetPointerX();
    [DllImport("uDesktopDuplication")]
    private static extern int GetPointerY();
    [DllImport("uDesktopDuplication")]
    private static extern int SetTexturePtr(IntPtr ptr);
    [DllImport("uDesktopDuplication")]
    private static extern IntPtr GetRenderEventFunc();

    public bool isPointerVisible = false;
    public int pointerX = 0;
    public int pointerY = 0;

    Coroutine renderCoroutine_ = null;

    void OnEnable()
    {
        var tex = new Texture2D(GetWidth(), GetHeight(), TextureFormat.BGRA32, false);
        GetComponent<Renderer>().sharedMaterial.mainTexture = tex;

        SetTexturePtr(tex.GetNativeTexturePtr());
        renderCoroutine_ = StartCoroutine(OnRender());
    }

    void OnDisable()
    {
        if (renderCoroutine_ != null) {
            StopCoroutine(renderCoroutine_);
            renderCoroutine_ = null;
        }
    }

    void Update()
    {
        isPointerVisible = IsPointerVisible();
        pointerX = GetPointerX();
        pointerY = GetPointerY();
    }

    IEnumerator OnRender()
    {
        for (;;) {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 0);
        }
    }
}