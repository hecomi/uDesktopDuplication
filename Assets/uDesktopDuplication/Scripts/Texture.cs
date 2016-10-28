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
            material_ = GetComponent<Renderer>().material;
            material_.mainTexture = monitor_.texture;
        }
    }

    public int monitorId
    { 
        get { return monitor.id; }
        set { monitor = Manager.monitors[Mathf.Clamp(value, 0, Manager.monitorCount - 1)]; }
    }

    public bool invertX = false;
    public bool invertY = false;

    private Material material_;

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
            material_.EnableKeyword("INVERT_X");
        } else {
            material_.DisableKeyword("INVERT_X");
        }

        if (invertY) {
            material_.EnableKeyword("INVERT_Y");
        } else {
            material_.DisableKeyword("INVERT_Y");
        }
    }
}

}