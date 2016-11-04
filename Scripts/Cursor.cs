﻿using UnityEngine;
using System.Collections.Generic;

namespace uDesktopDuplication
{

[AddComponentMenu("uDesktopDuplication/Cursor"), 
 RequireComponent(typeof(Texture))]
public class Cursor : MonoBehaviour
{
    Vector3 worldPosition { get; set; }

    private Texture uddTexture_;
    private Monitor monitor { get { return uddTexture_.monitor; } }

    private Texture2D currentTexture_;
    private Dictionary<Vector2, Texture2D> textures_ = new Dictionary<Vector2, Texture2D>();
    [SerializeField] Vector2 modelScale = Vector2.one;

    void Start()
    {
        uddTexture_ = GetComponent<Texture>();
    }

    void Update()
    {
        if (monitor.isCursorVisible) {
            UpdatePosition();
            UpdateTexture();
            UpdateMaterial();
        }
    }

    void UpdatePosition()
    {
        var x = (1f * monitor.pointerX / monitor.width  - 0.5f) * modelScale.x;
        var y = (1f * monitor.pointerY / monitor.height - 0.5f) * modelScale.y;
        var iy = uddTexture_.invertY ? -1 : +1;
        var localPos = transform.right * x + iy * transform.up * y;
        worldPosition = transform.TransformPoint(localPos);
    }

    void UpdateTexture()
    {
        var w = monitor.pointerShapeWidth;
        var h = monitor.pointerShapeHeight;
        if (w == 0 || h == 0) return;

        var scale = new Vector2(w, h);
        if (!textures_.ContainsKey(scale)) {
            var texture = new Texture2D(w, h, TextureFormat.BGRA32, false);
            texture.wrapMode = TextureWrapMode.Clamp;
            textures_.Add(scale, texture);
        }

        var pointerTexture = textures_[scale];
        monitor.UpdateCursorTexture(pointerTexture.GetNativeTexturePtr());
        uddTexture_.material.SetTexture("_CursorTex", pointerTexture);
    }

    void UpdateMaterial()
    {
        var x = (float)monitor.pointerX / monitor.width;
        var y = (float)monitor.pointerY / monitor.height;
        var w = (float)monitor.pointerShapeWidth  / monitor.width;
        var h = (float)monitor.pointerShapeHeight / monitor.height;
        uddTexture_.material.SetVector("_CursorPositionScale", new Vector4(x, y, w, h));
    }
}

}