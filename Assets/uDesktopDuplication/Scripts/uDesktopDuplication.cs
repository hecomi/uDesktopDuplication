using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;

namespace uDesktopDuplication
{

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

    private Material material_;
    public bool invertX = false;
    public bool invertY = false;

    public delegate void MouseMoveEventHandler(Vector2 pos);
    public MouseMoveEventHandler onMouseMove { get; set; }

    private Coroutine renderCoroutine_ = null;

    void OnEnable()
    {
        var tex = new Texture2D(GetWidth(), GetHeight(), TextureFormat.BGRA32, false);
        material_ = GetComponent<Renderer>().material;
        material_.mainTexture = tex;

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
        UpdateMouseEvent();
        UpdateMaterial();
    }

    void UpdateMouseEvent()
    {
        var isVisible = IsPointerVisible();
        if (isVisible && onMouseMove != null) {
            var x = GetPointerX();
            var y = GetPointerY();
            var w = GetWidth();
            var h = GetHeight();
            onMouseMove(new Vector2(2f * x / w - 1f, 1f - 2f * y / h));
        }
    }

    void UpdateMaterial()
    {
        if (invertX) {
            material_.EnableKeyword("INVERT_X");
        } else {
            material_.DisableKeyword("INVERT_X");
        }

        if (invertY) {
            material_.EnableKeyword("INVERT_Y");
        } else {
            material_.DisableKeyword("INVERT_Y");
        }
    }

    IEnumerator OnRender()
    {
        for (;;) {
            yield return new WaitForEndOfFrame();
            GL.IssuePluginEvent(GetRenderEventFunc(), 0);
        }
    }
}

}