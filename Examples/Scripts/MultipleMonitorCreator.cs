using UnityEngine;
using System.Collections.Generic;

public class MultipleMonitorCreator : MonoBehaviour
{
    [SerializeField] GameObject monitorPrefab;

    public class MonitorInfo
    {
        public GameObject gameObject { get; set; }
        public Quaternion originalRotation { get; set; }
        public uDesktopDuplication.Texture uddTexture { get; set; }
        public Vector3 meshBounds;
    }

   private List<MonitorInfo> monitors_ = new List<MonitorInfo>();
   public List<MonitorInfo> monitors { get { return monitors_; } }

    void Start()
    {
        Create();
    }

    void OnEnable()
    {
        uDesktopDuplication.Manager.onReinitialized += Recreate;
    }

    void OnDisable()
    {
        uDesktopDuplication.Manager.onReinitialized -= Recreate;
    }

    void Create()
    {
        // Sort monitors in coordinate order
        var monitors = uDesktopDuplication.Manager.monitors;
        monitors.Sort((a, b) => a.left - b.left);

        // Create monitors
        for (int i = 0 ; i < uDesktopDuplication.Manager.monitorCount; ++i) {
            // Create monitor obeject
            var go = Instantiate(monitorPrefab);
            go.name = "Monitor " + i;

            // Assign monitor
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;
            var monitor = texture.monitor;

            // Set width / height
            go.transform.localScale = new Vector3(monitor.widthMeter, go.transform.localScale.y, monitor.heightMeter);

            // Set parent as this object
            go.transform.SetParent(transform);

            // Calc actual size considering mesh size
            var bounds = go.GetComponent<MeshFilter>().sharedMesh.bounds.extents * 2f;

            // Save
            var info = new MonitorInfo();
            info.gameObject = go;
            info.originalRotation = go.transform.rotation;
            info.uddTexture = texture;
            info.meshBounds = bounds;
            monitors_.Add(info);
        }
    }

    void Clear()
    {
        foreach (var info in monitors_) {
            Destroy(info.gameObject);
        }
        monitors_.Clear();
    }

    void Recreate()
    {
        Clear();
        Create();
    }
}

