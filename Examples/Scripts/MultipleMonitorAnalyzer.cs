using UnityEngine;

[RequireComponent(typeof(MultipleMonitorCreator))]
public class MultipleMonitorAnalyzer : MonoBehaviour
{
    MultipleMonitorCreator creator_;
    Vector3 gazePoint_ = Vector3.zero;
    public Vector3 gazePoint 
    { 
        get { return gazePoint_; } 
        private set 
        {
            gazePoint_ += (value - gazePoint) * (gazePointFilter / (1f / 60 / Time.deltaTime));
        }
    }

    [Header("Filters")]
    [Range(0f, 1f)] public float gazePointFilter = 0.1f;
    [Range(0f, 1f)] public float moveRectFilter = 0.05f;
    [Range(0f, 1f)] public float mouseFilter = 0.05f;
    [Range(0f, 1f)] public float dirtyRectFilter = 0.01f;
    [Range(0f, 1f)] public float noEventFilter = 0.01f;
    [Range(0f, 1f)] public float velocityFilter = 0.1f;

    [Header("Debug")]
    [SerializeField] bool drawGazePoint;
    [SerializeField] bool drawAveragePos;
    [SerializeField] bool drawMoveRects;
    [SerializeField] bool drawDirtyRects;

    void Start()
    {
        creator_ = GetComponent<MultipleMonitorCreator>();
    }

    void Update()
    {
        var cursorMonitorId = uDesktopDuplication.Manager.cursorMonitorId;
        for (int i = 0; i < creator_.monitors.Count; ++i) {
            var info = creator_.monitors[i];
            var analyzer = 
                info.gameObject.GetComponent<GazePointAnalyzer>() ??
                info.gameObject.AddComponent<GazePointAnalyzer>();
            UpdateAnalyzer(analyzer);
            if (info.uddTexture.monitorId == cursorMonitorId) {
                gazePoint = analyzer.averagePos;
            }
        }

        if (drawGazePoint) DrawGazePoint();
    }

    void DrawGazePoint()
    {
        Debug.DrawLine(transform.position, gazePoint, Color.magenta);
    }

    void UpdateAnalyzer(GazePointAnalyzer analyzer)
    {
        analyzer.moveRectFilter = moveRectFilter;
        analyzer.mouseFilter = mouseFilter;
        analyzer.dirtyRectFilter = dirtyRectFilter;
        analyzer.noEventFilter = noEventFilter;
        analyzer.velocityFilter = velocityFilter;
        analyzer.drawAveragePos = drawAveragePos;
        analyzer.drawMoveRects = drawMoveRects;
        analyzer.drawDirtyRects = drawDirtyRects;
    }
}
