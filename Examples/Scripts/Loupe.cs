using UnityEngine;

public class Loupe : MonoBehaviour
{
    private uDesktopDuplication.Texture uddTexture_;
    public float zoom = 3f;
    public float aspect = 1f;

    void Start()
    {
        uddTexture_ = GetComponent<uDesktopDuplication.Texture>();
        uddTexture_.useClip = true;
    }

    void Update()
    {
        CheckVariables();

        if (uDesktopDuplication.Manager.cursorMonitorId < 0) return;
        uddTexture_.monitorId = uDesktopDuplication.Manager.cursorMonitorId;

        // To get other monitor textures, set dirty flag.
        foreach (var monitor in uDesktopDuplication.Manager.monitors) {
            monitor.CreateTexture();
            monitor.shouldBeUpdated = true;
        }

        var x = (float)uddTexture_.monitor.cursorX / uddTexture_.monitor.width;
        var y = (float)uddTexture_.monitor.cursorY / uddTexture_.monitor.height;
        var w = 1f / zoom;
        var h = w / aspect * uddTexture_.monitor.aspect;
        x = Mathf.Clamp(x - w / 2, 0f, 1f - w);
        y = Mathf.Clamp(y - h / 2, 0f, 1f - h);
        uddTexture_.clipPos = new Vector2(x, y);
        uddTexture_.clipScale = new Vector2(w, h);
    }

    void CheckVariables()
    {
        if (zoom < 1f) zoom = 1f;
        if (aspect < 0.01f) aspect = 0.01f;
    }
}

