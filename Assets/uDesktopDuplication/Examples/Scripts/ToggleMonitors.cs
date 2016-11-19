using UnityEngine;

public class ToggleMonitors : MonoBehaviour
{
    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Tab)) {
            var texture = GetComponent<uDesktopDuplication.Texture>();
            var id = texture.monitorId;
            var n = uDesktopDuplication.Manager.monitorCount;
            if (Input.GetKey(KeyCode.LeftShift) || Input.GetKey(KeyCode.RightShift)) {
                texture.monitorId = (id - 1 < 0) ? 0 : (id - 1);
            } else {
                texture.monitorId = (id + 1 >= n) ? (n - 1) : (id + 1);
            }
        }
    }
}

