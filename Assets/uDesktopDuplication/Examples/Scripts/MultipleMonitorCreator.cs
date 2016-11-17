using UnityEngine;
using System.Collections.Generic;
using MeshForwardDirection = uDesktopDuplication.Texture.MeshForwardDirection;

public class MultipleMonitorCreator : MonoBehaviour
{
    [Tooltip("Create monitors using this prefab.")]
    public GameObject monitorPrefab;

    public enum ScaleMode
    {
        Real,
        Fixed,
        Pixel,
    }

    [Tooltip("Real: DPI-based real scale \nFixed: Same width \nPixel: bigger if screen resolution is high.")]
    public ScaleMode scaleMode = ScaleMode.Fixed;

    [Tooltip("Use this scale as width if scaleMode is Fixed.")]
    public float scale = 0.5f;

    [Tooltip("Please specify the surface direction of the mesh (e.g. Plane => Y.)")]
    public MeshForwardDirection meshForwardDirection = MeshForwardDirection.Z;

    [Tooltip("Remove unsupported monitors automatically after removeWaitDuration.")]
    public bool removeIfUnsupported = true;

    [Tooltip("Remove unsupported monitors automatically after removeWaitDuration.")]
    public float removeWaitDuration = 5f;

    [Tooltip("Remove all childrens (for debug).")]
    public bool removeChildrenWhenClear = true;

    bool hasMonitorUnsupportStateChecked_ = false;
    float removeWaitTimer_ = 0f;

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
        if (!hasMonitorUnsupportStateChecked_) {
            removeWaitTimer_ += Time.deltaTime;
            if (removeWaitTimer_ > removeWaitDuration) {
                hasMonitorUnsupportStateChecked_ = true;
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
        hasMonitorUnsupportStateChecked_ = false;
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
            var mesh = go.GetComponent<MeshFilter>().sharedMesh;
            var aabbScale = mesh.bounds.size;
            aabbScale.y = Mathf.Max(aabbScale.y, aabbScale.x);
            aabbScale.z = Mathf.Max(aabbScale.z, aabbScale.x);
            mesh.bounds = new Bounds(mesh.bounds.center, aabbScale);

            // Assign monitor
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;
            var monitor = texture.monitor;

            // Set width / height
            float width = 1f, height = 1f;
            switch (scaleMode) {
                case ScaleMode.Real:
                    width = monitor.widthMeter;
                    height = monitor.heightMeter;
                    break;
                case ScaleMode.Fixed:
                    width = scale * (monitor.isHorizontal ? 1f : monitor.aspect);
                    height = scale * (monitor.isHorizontal ? 1f / monitor.aspect : 1f);
                    break;
                case ScaleMode.Pixel:
                    width = scale * (monitor.isHorizontal ? 1f : monitor.aspect) * ((float)monitor.width / 1920);
                    height = scale * (monitor.isHorizontal ? 1f / monitor.aspect : 1f) * ((float)monitor.width / 1920);
                    break;
            }
            if (meshForwardDirection == MeshForwardDirection.Y) {
                go.transform.localScale = new Vector3(width, go.transform.localScale.y, height);
            } else {
                go.transform.localScale = new Vector3(width, height, go.transform.localScale.z);
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
        if (removeChildrenWhenClear) {
            for (int i = 0; i < transform.childCount; ++i) {
                Destroy(transform.GetChild(i).gameObject);
            }
        }
        monitors_.Clear();
    }

    [ContextMenu("Recreate")]
    public void Recreate()
    {
        Clear();
        Create();
    }
}

