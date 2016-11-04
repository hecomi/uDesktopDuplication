using UnityEngine;

public class MultipleMonitorCreator : MonoBehaviour
{
    [SerializeField] GameObject monitorPrefab;
    [SerializeField] float width = 0.3f;
    [SerializeField] float margin = 1f;

    void Start()
    {
        var monitors = uDesktopDuplication.Manager.monitors;
        var n = monitors.Count;

        var totalWidth = 0f;
        for (int i = 0 ; i < n; ++i) {
            var go = Instantiate(monitorPrefab);
            go.name = "Monitor " + i;
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;
            var height = width / texture.monitor.aspect;
            go.transform.localScale = new Vector3(width, go.transform.localScale.y, height);
            go.transform.SetParent(transform);
            var scaleX = go.GetComponent<MeshFilter>().sharedMesh.bounds.extents.x * 2f;
            totalWidth += width * scaleX;
        }

        totalWidth += margin * (n - 1);
        var x = -totalWidth / 2;
        for (int i = 0 ; i < n; ++i) {
            var go = transform.FindChild("Monitor " + i);
            var halfScaleX = go.GetComponent<MeshFilter>().sharedMesh.bounds.extents.x;
            var halfWidth = width * halfScaleX;
            x += halfWidth;
            go.transform.localPosition = new Vector3(x, 0f, 0f);
            x += halfWidth + margin;
        }
    }
}

