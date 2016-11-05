using UnityEngine;
using System.Collections.Generic;

public class MultipleMonitorCreator : MonoBehaviour
{
    [SerializeField] GameObject monitorPrefab;
    [SerializeField] float width = 0.3f;
    [SerializeField] float margin = 1f;

    private List<GameObject> monitors_ = new List<GameObject>();

    void OnEnable()
    {
        Create();
        uDesktopDuplication.Manager.instance.onReinitialized += Recreate;
    }

    void OnDisable()
    {
        Clear();
        uDesktopDuplication.Manager.instance.onReinitialized -= Recreate;
    }

    void Create()
    {
        // Sort monitors in coordinate order
        var monitors = uDesktopDuplication.Manager.monitors;
        monitors.Sort((a, b) => a.left - b.left);

        // Create monitors
        var n = monitors.Count;
        var totalWidth = 0f;
        for (int i = 0 ; i < n; ++i) {
            // Create monitor obeject
            var go = Instantiate(monitorPrefab);
            go.name = "Monitor " + i;
            monitors_.Add(go);

            // Assign monitor
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;

            // Set width / height
            var isHorizontal = texture.monitor.isHorizontal;
            var w = isHorizontal ? width : (width * texture.monitor.aspect);
            var h = isHorizontal ? width / texture.monitor.aspect : width;
            go.transform.localScale = new Vector3(w, go.transform.localScale.y, h);

            // Set parent as this object
            go.transform.SetParent(transform);

            // Calc actual size considering mesh size
            var scaleX = go.GetComponent<MeshFilter>().sharedMesh.bounds.extents.x * 2f;
            totalWidth += w * scaleX;
        }

        // Set positions with margin
        totalWidth += margin * (n - 1);
        var x = -totalWidth / 2;
        foreach (var go in monitors_) {
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            var halfScaleX = go.GetComponent<MeshFilter>().sharedMesh.bounds.extents.x;
            var w = texture.monitor.isHorizontal ? width : (width * texture.monitor.aspect);
            var halfWidth = w * halfScaleX;
            x += halfWidth;
            go.transform.localPosition = new Vector3(x, 0f, 0f);
            x += halfWidth + margin;
        }
    }

    void Clear()
    {
        foreach (var go in monitors_) {
            Destroy(go);
        }
        monitors_.Clear();
    }

    void Recreate()
    {
        Clear();
        Create();
    }
}

