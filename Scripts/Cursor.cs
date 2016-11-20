using UnityEngine;

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

    void Start()
    {
        uddTexture_ = GetComponent<Texture>();
    }

    void Update()
    {
        if (monitor.isCursorVisible) {
            UpdatePosition();
        }
        UpdateCoords();
    }

    void UpdatePosition()
    {
        var x = (1f * monitor.cursorX / monitor.width  - 0.5f) * modelScale.x;
        var y = (1f * monitor.cursorY / monitor.height - 0.5f) * modelScale.y;
        var iy = uddTexture_.invertY ? -1 : +1;
        var localPos = transform.right * x + iy * transform.up * y;
        worldPosition = transform.TransformPoint(localPos);
    }

    void UpdateCoords()
    {
        var x = monitor.isCursorVisible ? (float)monitor.cursorX / monitor.width : -9999f;
        var y = monitor.isCursorVisible ? (float)monitor.cursorY / monitor.height : -9999f;
        coord = new Vector2(x, y);
    }
}

}