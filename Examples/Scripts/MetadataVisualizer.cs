using UnityEngine;
using MeshForwardDirection = uDesktopDuplication.Texture.MeshForwardDirection;

public class MetadataVisualizer : MonoBehaviour
{
    uDesktopDuplication.Texture uddTexture_;

    [SerializeField] bool drawMoveRectDest;
    [SerializeField] bool drawMoveRectSource;
    [SerializeField] bool drawDirtyRect;

    void Start()
    {
        uddTexture_ = GetComponent<uDesktopDuplication.Texture>();
    }

    public Vector3 GetWorldPositionFromCoord(int u, int v)
    {
        var monitor = uddTexture_.monitor;

        // mesh & scale information
        var mesh = GetComponent<MeshFilter>().sharedMesh;
        var width  = transform.localScale.x * (mesh.bounds.extents.x * 2f);
        var height = transform.localScale.y * (mesh.bounds.extents.y * 2f);

        // local position (scale included).
        var x =  (float)(u - monitor.width  / 2) / monitor.width;
        var y = -(float)(v - monitor.height / 2) / monitor.height;
        var localPos = new Vector3(width * x, height * y, 0f);

        // bend
        if (uddTexture_.bend) {
            var radius = uddTexture_.radius;
            var angle = localPos.x / radius;
            if (uddTexture_.meshForwardDirection == MeshForwardDirection.Y) {
                localPos.y -= radius * (1f - Mathf.Cos(angle));
            } else {
                localPos.z -= radius * (1f - Mathf.Cos(angle));
            }
            localPos.x  = radius * Mathf.Sin(angle);
        }

        // to world position
        return transform.position + (transform.rotation * localPos);
    }

    void Update()
    {
        UpdateMoveRects();
        UpdateDirtyRects();
    }

    void DrawRect(uDesktopDuplication.RECT rect, Color color)
    {
        var p0 = GetWorldPositionFromCoord(rect.left,  rect.top);
        var p1 = GetWorldPositionFromCoord(rect.right, rect.top);
        var p2 = GetWorldPositionFromCoord(rect.right, rect.bottom);
        var p3 = GetWorldPositionFromCoord(rect.left,  rect.bottom);

        Debug.DrawLine(p0, p1, color);
        Debug.DrawLine(p1, p2, color);
        Debug.DrawLine(p2, p3, color);
        Debug.DrawLine(p3, p0, color);
    }

    void UpdateMoveRects()
    {
        if (drawMoveRectSource) {
            foreach (var rect in uddTexture_.monitor.moveRects) {
                DrawRect(rect.source, Color.blue);
            }
        }
        if (drawMoveRectDest) {
            foreach (var rect in uddTexture_.monitor.moveRects) {
                DrawRect(rect.destination, Color.green);
            }
        }
    }

    void UpdateDirtyRects()
    {
        if (drawDirtyRect) {
            foreach (var rect in uddTexture_.monitor.dirtyRects) {
                DrawRect(rect, Color.red);
            }
        }
    }
}
