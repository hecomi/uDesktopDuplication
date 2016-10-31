using UnityEngine;
using System.Collections.Generic;

namespace uDesktopDuplication
{

[AddComponentMenu("uDesktopDuplication/Cursor"), 
 RequireComponent(typeof(Texture))]
public class Cursor : MonoBehaviour
{
    [SerializeField] Vector2 modelScale = Vector2.one;

    Vector3 worldPosition { get; set; }

    private Texture uddTexture_;
    private Monitor monitor { get { return uddTexture_.monitor; } }

    private Texture2D pointerTexture_;
    private Material cursorMaterial_;

    private Dictionary<Vector2, Texture2D> pointerTextures_ = new Dictionary<Vector2, Texture2D>();

    void OnEnable()
    {
        uddTexture_ = GetComponent<Texture>();
    }

    void Start()
    {
        var clone = new GameObject(name + " Cursor");
        clone.transform.SetParent(transform);
        clone.transform.localPosition = Vector3.zero;
        clone.transform.localRotation = Quaternion.identity;
        clone.transform.localScale = Vector3.one;

        var filter = clone.AddComponent<MeshFilter>();
        filter.mesh = GetComponent<MeshFilter>().sharedMesh;

        var renderer = clone.AddComponent<MeshRenderer>();
        var shader = Shader.Find("uDesktopDuplication/Cursor");
        cursorMaterial_ = new Material(shader);
        renderer.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off;
        renderer.receiveShadows = false;
        renderer.motionVectors = false;
        renderer.material = cursorMaterial_;
    }

    void Update()
    {
        if (monitor.isPointerVisible) {
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
        var scale = new Vector2(monitor.pointerShapeWidth, monitor.pointerShapeHeight);
        if (!pointerTextures_.ContainsKey(scale)) {
            var texture = new Texture2D(monitor.pointerShapeWidth, monitor.pointerShapeHeight, TextureFormat.BGRA32, false);
            texture.wrapMode = TextureWrapMode.Clamp;
            pointerTextures_.Add(scale, texture);
        }
        var pointerTexture = pointerTextures_[scale];
        monitor.UpdatePointerTexture(pointerTexture.GetNativeTexturePtr());
        cursorMaterial_.SetTexture("_MainTex", pointerTexture);
    }

    void UpdateMaterial()
    {
        if (uddTexture_.invertX) {
            cursorMaterial_.EnableKeyword("INVERT_X");
        } else {
            cursorMaterial_.DisableKeyword("INVERT_X");
        }

        if (uddTexture_.invertY) {
            cursorMaterial_.EnableKeyword("INVERT_Y");
        } else {
            cursorMaterial_.DisableKeyword("INVERT_Y");
        }

        if (monitor.isVertical) {
            cursorMaterial_.EnableKeyword("VERTICAL");
        } else {
            cursorMaterial_.DisableKeyword("VERTICAL");
        }

        var x = (float)monitor.pointerX / monitor.width;
        var y = (float)monitor.pointerY / monitor.height;
        var w = (float)monitor.pointerShapeWidth  / monitor.width;
        var h = (float)monitor.pointerShapeHeight / monitor.height;
        cursorMaterial_.SetVector("_PositionScale", new Vector4(x, y, w, h));
    }
}

}