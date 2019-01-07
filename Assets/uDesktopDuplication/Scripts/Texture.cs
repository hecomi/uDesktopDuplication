using UnityEngine;

namespace uDesktopDuplication
{

[RequireComponent(typeof(Renderer))]
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

    int clipPositionScaleKey_;

    void Awake()
    {
        clipPositionScaleKey_ = Shader.PropertyToID("_ClipPositionScale");
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
        if (monitor != null) {
            monitor.shouldBeUpdated = true;
        }
    }

    void Reinitialize()
    {
        // Monitor instance is released here when initialized.
        monitor = Manager.GetMonitor(lastMonitorId_);
    }

    void UpdateMaterial()
    {
        width = transform.localScale.x;

        if (monitor != null) {
            rotation = monitor.rotation;
        }

        material.SetVector(clipPositionScaleKey_, new Vector4(clipPos.x, clipPos.y, clipScale.x, clipScale.y));
    }

    public Vector3 GetWorldPositionFromCoord(Vector2 coord)
    {
        return GetWorldPositionFromCoord((int)coord.x, (int)coord.y);
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

    public struct RayCastResult
    {
        public bool hit;
        public Texture texture;
        public Vector3 position;
        public Vector3 normal;
        public Vector2 coords;
        public Vector2 desktopCoord;
    }

    static readonly RayCastResult raycastFailedResult = new RayCastResult {
        hit          = false,
        texture      = null,
        position     = Vector3.zero,
        normal       = Vector3.forward,
        coords       = Vector2.zero,
        desktopCoord = Vector2.zero,
    };

    // This function can be used only for vertical (= MeshForwardDirection.Z) plane.
    public RayCastResult RayCast(Vector3 from, Vector3 dir)
    {
        var r = radius;
        var center = transform.position - transform.forward * r;

        // Localize the start point of the ray and the direction.
        var trs = Matrix4x4.TRS(center, transform.rotation, Vector3.one);
        var invTrs = trs.inverse;
        Vector3 localFrom = invTrs.MultiplyPoint3x4(from);
        Vector3 localDir = invTrs.MultiplyVector(dir).normalized;

        // Calculate the intersection points of circle and line on the X-Z plane.
        var a = localDir.z / localDir.x;
        var b = localFrom.z - a * localFrom.x;

        var aa = a * a;
        var bb = b * b;
        var ab = a * b;
        var rr = r * r;

        var s = aa * rr - bb + rr;
        if (s < 0f) {
            return raycastFailedResult;
        }
        s = Mathf.Sqrt(s);

        var t = aa + 1;

        var lx0 = (-s - ab) / t;
        var lz0 = (b - a * s) / t;
        var to0 = new Vector3(lx0, 0f, lz0);

        var lx1 = (s - ab) / t;
        var lz1 = (a * s + b) / t;
        var to1 = new Vector3(lx1, 0f, lz1);

        var to = (Vector3.Dot(localDir, to0) > 0f) ? to0 : to1;

        // Check if the point is inner angle of mesh width.
        var toAngle = Mathf.Atan2(to.x, to.z);
        var halfWidthAngle = (worldWidth / radius) * 0.5f;
        if (Mathf.Abs(toAngle) > halfWidthAngle) {
            return raycastFailedResult;
        }

        // Calculate the intersection points on XZ-Y plane.
        var v = to - localFrom;
        var l = Mathf.Sqrt(Mathf.Pow(v.x, 2f) + Mathf.Pow(v.z, 2f));
        var ly = localFrom.y + l * localDir.y / Mathf.Sqrt(Mathf.Pow(localDir.x, 2f) + Mathf.Pow(localDir.z, 2f));

        // Check if the point is inner mesh height.
        var halfHeight = worldHeight * 0.5f;
        if (Mathf.Abs(ly) > halfHeight) {
            return raycastFailedResult;
        }

        // Check hit position is in the range of the ray.
        to.y = ly;
        var hitPos = trs.MultiplyPoint(to);

        if ((hitPos - from).magnitude > dir.magnitude) {
            return raycastFailedResult;
        }

        // Calculate coordinates.
        var coordX = toAngle / halfWidthAngle * 0.5f;
        var coordY = ly / halfHeight * 0.5f;

        // Zoom
        if (useClip) {
            coordX = clipPos.x + (0.5f + coordX) * clipScale.x;
            coordX -= Mathf.Floor(coordX);
            coordX -= 0.5f;

            coordY = (1f - clipPos.y) + (coordY - 0.5f) * clipScale.y;
            coordY -= Mathf.Floor(coordY);
            coordY -= 0.5f;
        }

        // Desktop position
        int desktopX = monitor.left + (int)((coordX + 0.5f) * monitor.width);
        int desktopY = monitor.top + (int)((0.5f - coordY) * monitor.height);

        // Calculate normal.
        var normal = new Vector3(-to.x, 0f, -to.z);

        // Result
        return new RayCastResult {
            hit = true,
            texture = this,
            position = trs.MultiplyPoint(to),
            normal = trs.MultiplyVector(normal).normalized,
            coords = new Vector2(coordX, coordY),
            desktopCoord = new Vector2(desktopX, desktopY)
        };
    }

    public static RayCastResult RayCastAll(Vector3 from, Vector3 dir)
    {
        foreach (var uddTexture in GameObject.FindObjectsOfType<uDesktopDuplication.Texture>()) {
            var result = uddTexture.RayCast(from, dir);
            if (result.hit) return result;
        }
        return raycastFailedResult;
    }
}

}