using UnityEngine;

public class ToggleMonitors : MonoBehaviour
{
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Tab)) {
            var texture = GetComponent<uDesktopDuplication.Texture>();
            if (Input.GetKey(KeyCode.LeftShift) || Input.GetKey(KeyCode.RightShift)) {
                texture.monitorId--;
            } else {
                texture.monitorId++;
            }
        }
    }
}

