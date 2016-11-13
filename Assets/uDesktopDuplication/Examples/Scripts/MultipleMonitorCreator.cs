using UnityEngine;
using System.Collections.Generic;

public class MultipleMonitorCreator : MonoBehaviour
{
    [SerializeField] GameObject monitorPrefab;
    [SerializeField] bool removeIfUnsupported = true;
    [SerializeField] float removeWaitDuration = 5f;
    bool hasMonitorUnsupportStateChecked = false;
    float removeWaitTimer_ = 0f;

    public enum MeshForwardDirection { Y, Z }
    [Tooltip("Please specify the upper vector direction of the mesh (e.g. Plane's upper direction is Y.)")]
    public MeshForwardDirection meshForwardDirection = MeshForwardDirection.Z;

    public class MonitorInfo
    {
        public GameObject gameObject { get; set; }
        public Quaternion originalRotation { get; set; }
        public uDesktopDuplication.Texture uddTexture { get; set; }
        public Mesh mesh { get; set; }
    }

    private List<MonitorInfo> monitors_ = new List<MonitorInfo>();
    public List<MonitorInfo> monitors { get { return monitors_; } }

    void Start()
    {
        uDesktopDuplication.Manager.CreateInstance();
        Create();
    }

    void Update()
    {
        if (removeIfUnsupported) {
            RemoveUnsupportedDisplayAfterRemoveTimer();
        }

        if (uDesktopDuplication.Manager.monitorCount != monitors.Count) {
            Recreate();
        }
    }

    void OnEnable()
    {
        uDesktopDuplication.Manager.onReinitialized += Recreate;
    }

    void OnDisable()
    {
        uDesktopDuplication.Manager.onReinitialized -= Recreate;
    }

    void RemoveUnsupportedDisplayAfterRemoveTimer()
    {
        if (!hasMonitorUnsupportStateChecked) {
            removeWaitTimer_ += Time.deltaTime;
            if (removeWaitTimer_ > removeWaitDuration) {
                hasMonitorUnsupportStateChecked = true;
                foreach (var info in monitors_) {
                    if (info.uddTexture.monitor.state == uDesktopDuplication.MonitorState.Unsupported) {
                        Destroy(info.gameObject);
                    }
                }
                monitors_.RemoveAll(info => info.uddTexture.monitor.state == uDesktopDuplication.MonitorState.Unsupported);
            }
        }
    }

    void ResetRemoveTimer()
    {
        hasMonitorUnsupportStateChecked = false;
        removeWaitTimer_ = 0f;
    }

    void Create()
    {
        ResetRemoveTimer();

        // Create monitors
        for (int i = 0; i < uDesktopDuplication.Manager.monitorCount; ++i) {
            // Create monitor obeject
            var go = Instantiate(monitorPrefab);
            go.name = "Monitor " + i;

            // Expand AABB
            var mesh = go.GetComponent<MeshFilter>().mesh; // clone
            var aabbScale = mesh.bounds.size;
            aabbScale.y = Mathf.Max(aabbScale.y, aabbScale.x);
            aabbScale.z = Mathf.Max(aabbScale.z, aabbScale.x);
            mesh.bounds = new Bounds(mesh.bounds.center, aabbScale);

            // Assign monitor
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;
            var monitor = texture.monitor;

            // Set width / height
            if (meshForwardDirection == MeshForwardDirection.Y) {
                go.transform.localScale = new Vector3(monitor.widthMeter, go.transform.localScale.y, monitor.heightMeter);
            } else {
                go.transform.localScale = new Vector3(monitor.widthMeter, monitor.heightMeter, go.transform.localScale.z);
            }

            // Set parent as this object
            go.transform.SetParent(transform);

            // Save
            var info = new MonitorInfo();
            info.gameObject = go;
            info.originalRotation = go.transform.rotation;
            info.uddTexture = texture;
            info.mesh = mesh;
            monitors_.Add(info);
        }

        // Sort monitors in coordinate order
        monitors_.Sort((a, b) => a.uddTexture.monitor.left - b.uddTexture.monitor.left);
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

