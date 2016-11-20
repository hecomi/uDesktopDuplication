using UnityEngine;
using UnityEngine.Assertions;
using System.Collections.Generic;

namespace uDesktopDuplication
{

[AddComponentMenu("uDesktopDuplication/Cursor"), RequireComponent(typeof(Texture))]
public class Cursor : MonoBehaviour
{
    [SerializeField] Vector2 modelScale = Vector2.one;

    public Vector3 worldPosition { get; set; }
    public Vector2 coord { get; set; }

    private Texture uddTexture_;
    private Monitor monitor { get { return uddTexture_.monitor; } }

    struct TextureInfo
    {
        public Texture2D texture;
        public System.IntPtr ptr;
        public TextureInfo(Texture2D texture)
        {
            this.texture = texture;
            this.ptr = texture.GetNativeTexturePtr();
        }
    }
    private Dictionary<int, TextureInfo> textures_ = new Dictionary<int, TextureInfo>();

    void Start()
    {
        uddTexture_ = GetComponent<Texture>();
    }

    void Update()
    {
        if (monitor.isCursorVisible) {
            UpdatePosition();
            UpdateTexture();
        }
        UpdateMaterial();
    }

    void UpdatePosition()
    {
        var x = (1f * monitor.cursorX / monitor.width  - 0.5f) * modelScale.x;
        var y = (1f * monitor.cursorY / monitor.height - 0.5f) * modelScale.y;
        var iy = uddTexture_.invertY ? -1 : +1;
        var localPos = transform.right * x + iy * transform.up * y;
        worldPosition = transform.TransformPoint(localPos);
    }

    void UpdateTexture()
    {
        var w = monitor.cursorShapeWidth;
        var h = monitor.cursorShapeHeight;
        if (w == 0 || h == 0) return;

        var key = w + h * 100;
        if (!textures_.ContainsKey(key)) {
            var texture = new Texture2D(w, h, TextureFormat.BGRA32, false);
            texture.wrapMode = TextureWrapMode.Clamp;
            textures_.Add(key, new TextureInfo(texture));
        }

        var cursorTexture = textures_[key];
        Assert.IsNotNull(cursorTexture.texture);
        monitor.GetCursorTexture(cursorTexture.ptr);
        uddTexture_.material.SetTexture("_CursorTex", cursorTexture.texture);
    }

    void UpdateMaterial()
    {
        var x = monitor.isCursorVisible ? (float)monitor.cursorX / monitor.width : -9999f;
        var y = monitor.isCursorVisible ? (float)monitor.cursorY / monitor.height : -9999f;
        var w = (float)monitor.cursorShapeWidth  / monitor.width;
        var h = (float)monitor.cursorShapeHeight / monitor.height;
        uddTexture_.material.SetVector("_CursorPositionScale", new Vector4(x, y, w, h));

        coord = new Vector2(x, y);
    }
}

}