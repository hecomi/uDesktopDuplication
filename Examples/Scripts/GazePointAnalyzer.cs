using UnityEngine;

[RequireComponent(typeof(uDesktopDuplication.Texture))]
public class GazePointAnalyzer : MonoBehaviour
{
    uDesktopDuplication.Texture uddTexture_;

    [Tooltip("This needs a small calculation cost.")]
    public bool calcAveragePos = true;

    private Vector2 averageCoord_ = Vector2.zero;
    private Vector2 averageCoordVelocity_ = Vector2.zero;
    public Vector3 averagePos
    {
        get { return GetWorldPositionFromCoord((int)averageCoord_.x, (int)averageCoord_.y); }
    }

    public Vector3 cursorPos
    {
        get { return GetWorldPositionFromCoord(uddTexture_.monitor.cursorX, uddTexture_.monitor.cursorY); }
    }
    private Vector2 preCursorCoord_ = Vector2.zero;

    [Header("Filters")]
    [Range(0f, 1f)] public float moveRectFilter = 0.05f;
    [Range(0f, 1f)] public float mouseFilter = 0.05f;
    [Range(0f, 1f)] public float dirtyRectFilter = 0.01f;
    [Range(0f, 1f)] public float noEventFilter = 0.01f;
    [Range(0f, 1f)] public float velocityFilter = 0.1f;

    [Header("Debug")]
    public bool drawAveragePos;
    public bool drawCursorPos;
    public bool drawMoveRects;
    public bool drawDirtyRects;

    void Start()
    {
        uddTexture_ = GetComponent<uDesktopDuplication.Texture>();
        averageCoord_ = new Vector2(uddTexture_.monitor.width / 2, uddTexture_.monitor.height / 2);
    }

    public Vector3 GetWorldPositionFromCoord(int u, int v)
    {
        return uddTexture_.GetWorldPositionFromCoord(u, v);
    }

    void CalcAveragePos()
    {
        if (!calcAveragePos) return;

        var coord = Vector2.zero;
        var monitor = uddTexture_.monitor;
        var cursorCoord = new Vector2(monitor.cursorX, monitor.cursorY);
        var filter = 0f;

        // move rect
        if (monitor.moveRectCount > 0) {
            foreach (var rects in monitor.moveRects) {
                var rect = rects.destination;
                var center = new Vector2(
                    (rect.right  + rect.left) / 2, 
                    (rect.bottom + rect.top)  / 2);
                coord += center;
            }
            coord /= monitor.moveRectCount;
            filter = moveRectFilter;
        } 
        // mouse
        else if (
            monitor.isCursorVisible && 
            cursorCoord != preCursorCoord_ &&
            (cursorCoord - preCursorCoord_).magnitude > 5 &&
            monitor.cursorX >= 0 && 
            monitor.cursorY >= 0) 
        {
            coord = cursorCoord;
            filter = mouseFilter;
        } 
        // dirty rect
        else if (monitor.dirtyRectCount > 0) {
            var totalWeights = 0f;
            foreach (var rect in monitor.dirtyRects) {
                var center = new Vector2(
                    (rect.right  + rect.left) / 2, 
                    (rect.bottom + rect.top)  / 2);
                var weight = 1f / Mathf.Sqrt((rect.right - rect.left) * (rect.bottom - rect.top));
                coord += center * weight;
                totalWeights += weight;
            }
            coord /= totalWeights;
            filter = dirtyRectFilter;
        } 
        // no event
        else {
            coord = new Vector2(monitor.width / 2, monitor.height / 2);
            filter = noEventFilter;
        }

        var cf = (filter / ((1f / 60) / Time.deltaTime));
        var vf = (velocityFilter / ((1f / 60) / Time.deltaTime));
        var targetCoord = averageCoord_ + (coord - averageCoord_) * cf;
        var targetVelocity = targetCoord - averageCoord_;

        if (float.IsNaN(targetCoord.x) || float.IsNaN(targetCoord.y)) return;
        if (float.IsNaN(targetVelocity.x) || float.IsNaN(targetVelocity.y)) return;

        averageCoordVelocity_ += (targetVelocity - averageCoordVelocity_) * vf;
        averageCoord_ += averageCoordVelocity_;
        averageCoord_.x = Mathf.Clamp(averageCoord_.x, 0, monitor.width);
        averageCoord_.y = Mathf.Clamp(averageCoord_.y, 0, monitor.height);

        preCursorCoord_ = cursorCoord;
    }

    void Update()
    {
        CalcAveragePos();
        DebugDraw();
    }

    void DebugDraw()
    {
        if (drawAveragePos) DrawAveragePos();
        if (drawCursorPos)  DrawCursorPos();
        if (drawDirtyRects) DrawDirtyRects();
        if (drawMoveRects)  DrawMoveRects();
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

    void DrawAveragePos()
    {
        Debug.DrawLine(transform.position, averagePos, Color.yellow);
    }

    void DrawCursorPos()
    {
        Debug.DrawLine(transform.position, cursorPos, Color.grey);
    }

    void DrawMoveRects()
    {
        foreach (var rect in uddTexture_.monitor.moveRects) {
            DrawRect(rect.source, Color.blue);
            DrawRect(rect.destination, Color.green);
        }
    }

    void DrawDirtyRects()
    {
        foreach (var rect in uddTexture_.monitor.dirtyRects) {
            DrawRect(rect, Color.red);
        }
    }
}
