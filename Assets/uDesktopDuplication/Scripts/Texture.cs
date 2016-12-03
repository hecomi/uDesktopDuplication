using UnityEngine;

namespace uDesktopDuplication
{

[AddComponentMenu("uDesktopDuplication/Texture")] 
public class Texture : MonoBehaviour
{
    Monitor monitor_;
    public Monitor monitor 
    { 
        get { return monitor_; }
        set 
        { 
            monitor_ = value;
            if (monitor_ != null) {
                material.mainTexture = monitor_.texture;
                width = transform.localScale.x;
                rotation = monitor.rotation;
                invertX = invertX_;
                invertY = invertY_;
                useClip = useClip_;
            }
        }
    }

    int lastMonitorId_ = 0;
    public int monitorId
    { 
        get { return monitor.id; }
        set { monitor = Manager.GetMonitor(value); }
    }

    [SerializeField] bool invertX_ = false;
    public bool invertX
    {
        get 
        {
            return invertX_;
        }
        set 
        {
            invertX_ = value;
            if (invertX_) {
                material.EnableKeyword("INVERT_X");
            } else {
                material.DisableKeyword("INVERT_X");
            }
        }
    }

    [SerializeField] bool invertY_ = false;
    public bool invertY
    {
        get 
        {
            return invertY_;
        }
        set 
        {
            invertY_ = value;
            if (invertY_) {
                material.EnableKeyword("INVERT_Y");
            } else {
                material.DisableKeyword("INVERT_Y");
            }
        }
    }

    public MonitorRotation rotation
    {
        get
        {
            return monitor.rotation;
        }
        private set
        {
            switch (value)
            {
                case MonitorRotation.Identity:
                    material.DisableKeyword("ROTATE90");
                    material.DisableKeyword("ROTATE180");
                    material.DisableKeyword("ROTATE270");
                    break;
                case MonitorRotation.Rotate90:
                    material.EnableKeyword("ROTATE90");
                    material.DisableKeyword("ROTATE180");
                    material.DisableKeyword("ROTATE270");
                    break;
                case MonitorRotation.Rotate180:
                    material.DisableKeyword("ROTATE90");
                    material.EnableKeyword("ROTATE180");
                    material.DisableKeyword("ROTATE270");
                    break;
                case MonitorRotation.Rotate270:
                    material.DisableKeyword("ROTATE90");
                    material.DisableKeyword("ROTATE180");
                    material.EnableKeyword("ROTATE270");
                    break;
                default:
                    break;
            }
        }
    }

    [SerializeField] bool useClip_ = false;
    public Vector2 clipPos = Vector2.zero;
    public Vector2 clipScale = new Vector2(0.2f, 0.2f);
    public bool useClip
    {
        get
        {
            return useClip_;
        }
        set
        {
            useClip_ = value;
            if (useClip_) {
                material.EnableKeyword("USE_CLIP");
            } else {
                material.DisableKeyword("USE_CLIP");
            }
        }
    }

    public bool bend
    {
        get 
        {
            return material.GetInt("_Bend") != 0;
        }
        set 
        {
            if (value) {
                material.EnableKeyword("BEND_ON");
                material.SetInt("_Bend", 1);
            } else {
                material.DisableKeyword("BEND_ON");
                material.SetInt("_Bend", 0);
            }
        }
    }

    public enum MeshForwardDirection
    {
        Y = 0,
        Z = 1,
    }
    public MeshForwardDirection meshForwardDirection
    {
        get 
        {
            return (MeshForwardDirection)material.GetInt("_Forward");
        }
        set 
        {
            switch (value) {
                case MeshForwardDirection.Y:
                    material.SetInt("_Forward", 0);
                    material.EnableKeyword("_FORWARD_Y");
                    material.DisableKeyword("_FORWARD_Z");
                    break;
                case MeshForwardDirection.Z:
                    material.SetInt("_Forward", 1);
                    material.DisableKeyword("_FORWARD_Y");
                    material.EnableKeyword("_FORWARD_Z");
                    break;
            }
        }
    }

    public enum Culling
    {
        Off   = 0,
        Front = 1,
        Back  = 2,
    }
    public Culling culling
    {
        get 
        {
            return (Culling)material.GetInt("_Cull");
        }
        set 
        {
            material.SetInt("_Cull", (int)value);
        }
    }

    public float radius
    {
        get { return material.GetFloat("_Radius"); }
        set { material.SetFloat("_Radius", value); }
    }

    public float width
    {
        get { return material.GetFloat("_Width"); }
        set { material.SetFloat("_Width", value); }
    }

    public float thickness
    {
        get { return material.GetFloat("_Thickness"); }
        set { material.SetFloat("_Thickness", value); }
    }

    Material material_;
    public Material material
    {
        get
        {
            if (Application.isPlaying) {
                return material_ ?? (material_ = GetComponent<Renderer>().material); // clone
            } else {
                return GetComponent<Renderer>().sharedMaterial;
            }
        }
    }

    Mesh mesh
    {
        get 
        { 
            return GetComponent<MeshFilter>().sharedMesh; 
        }
    }
    public float worldWidth
    {
        get
        {
            return transform.localScale.x * (mesh.bounds.extents.x * 2f);
        }
    }
    public float worldHeight
    {
        get
        {
            return transform.localScale.y * (mesh.bounds.extents.y * 2f);
        }
    }

    void OnEnable()
    {
        if (monitor == null) {
            monitor = Manager.primary;
        }
        Manager.onReinitialized += Reinitialize;
    }

    void OnDisable()
    {
        Manager.onReinitialized -= Reinitialize;
    }

    void Update()
    {
        KeepMonitor();
        RequireUpdate();
        UpdateMaterial();
    }

    void KeepMonitor()
    {
        if (monitor == null) {
            Reinitialize();
        } else {
            lastMonitorId_ = monitorId;
        }
    }

    void RequireUpdate()
    {
        monitor.shouldBeUpdated = true;
    }

    void Reinitialize()
    {
        // Monitor instance is released here when initialized.
        monitor = Manager.GetMonitor(lastMonitorId_);
    }

    void UpdateMaterial()
    {
        width = transform.localScale.x;
        rotation = monitor.rotation;
        material.SetVector("_ClipPositionScale", new Vector4(clipPos.x, clipPos.y, clipScale.x, clipScale.y));
    }

    public Vector3 GetWorldPositionFromCoord(int u, int v)
    {
        // Local position (scale included).
        var x =  (float)(u - monitor.width  / 2) / monitor.width;
        var y = -(float)(v - monitor.height / 2) / monitor.height;
        var localPos = new Vector3(worldWidth * x, worldHeight * y, 0f);

        // Bending
        if (bend) {
            var angle = localPos.x / radius;
            if (meshForwardDirection == MeshForwardDirection.Y) {
                localPos.y -= radius * (1f - Mathf.Cos(angle));
            } else {
                localPos.z -= radius * (1f - Mathf.Cos(angle));
            }
            localPos.x  = radius * Mathf.Sin(angle);
        }

        // To world position
        return transform.position + (transform.rotation * localPos);
    }
}

}