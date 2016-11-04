using UnityEngine;

namespace uDesktopDuplication
{

[AddComponentMenu("uDesktopDuplication/Texture")] 
public class Texture : MonoBehaviour
{
    private Monitor monitor_;
    public Monitor monitor 
    { 
        get { return monitor_; }
        set 
        { 
            monitor_ = value;
            material = GetComponent<Renderer>().material;
            material.mainTexture = monitor_.texture;
        }
    }

    public int monitorId
    { 
        get { return monitor.id; }
        set { monitor = Manager.monitors[Mathf.Clamp(value, 0, Manager.monitorCount - 1)]; }
    }

    public bool invertX = false;
    public bool invertY = false;

    public Material material
    {
        get;
        private set;
    }

    void Awake()
    {
        if (!GetComponent<Cursor>())
        {
            gameObject.AddComponent<Cursor>();
        }
    }

    void OnEnable()
    {
        if (monitor == null) {
            monitor = Manager.primary;
        }
    }

    void Update()
    {
        monitor.shouldBeUpdated = true;
        UpdateMaterial();
    }

    void UpdateMaterial()
    {
        if (invertX) {
            material.EnableKeyword("INVERT_X");
        } else {
            material.DisableKeyword("INVERT_X");
        }

        if (invertY) {
            material.EnableKeyword("INVERT_Y");
        } else {
            material.DisableKeyword("INVERT_Y");
        }

        if (monitor.isVertical) {
            material.EnableKeyword("VERTICAL");
        } else {
            material.DisableKeyword("VERTICAL");
        }
    }
}

}