using UnityEngine;

[RequireComponent(typeof(uDesktopDuplication.Texture))]
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

    void LateUpdate()
    {
        CheckVariables();

        if (uDesktopDuplication.Manager.cursorMonitorId < 0) return;
        uddTexture_.monitorId = uDesktopDuplication.Manager.cursorMonitorId;

        // To get other monitor textures, set dirty flag.
        foreach (var target in uDesktopDuplication.Manager.monitors) {
            target.CreateTextureIfNeeded();
            target.shouldBeUpdated = true;
        }

        var monitor = uddTexture_.monitor;
        var hotspotX = uDesktopDuplication.Lib.GetCursorHotSpotX();
        var hotspotY = uDesktopDuplication.Lib.GetCursorHotSpotY();
        var x = monitor.isCursorVisible ? 
            (float)(monitor.cursorX + hotspotX) / monitor.width :
            (float)monitor.systemCursorX / monitor.width;
        var y = monitor.isCursorVisible ? 
            (float)(monitor.cursorY + hotspotY) / monitor.height :
            (float)monitor.systemCursorY / monitor.height;
        var w = 1f / zoom;
        var h = w / aspect * monitor.aspect;
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

