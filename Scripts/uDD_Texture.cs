using UnityEngine;

namespace uDesktopDuplication
{

public class uDD_Texture : MonoBehaviour
{
    public uDD_Monitor monitor { get; set; }

    public bool invertX = false;
    public bool invertY = false;

    private Material material_;

    void OnEnable()
    {
        if (monitor == null) {
            monitor = uDD_Manager.primary;
        }
        material_ = GetComponent<Renderer>().material;
        material_.mainTexture = monitor.texture;
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