using UnityEngine;

public class GetPixelsExample : MonoBehaviour
{
    [SerializeField] uDesktopDuplication.Texture uddTexture;

    [SerializeField] int x = 100;
    [SerializeField] int y = 100;
    const int width = 64;
    const int height = 32;
    
    public Texture2D texture;
    Color32[] colors = new Color32[width * height];

    void Start()
    {
        texture = new Texture2D(width, height, TextureFormat.ARGB32, false);
        GetComponent<Renderer>().material.mainTexture = texture;
    }

    void Update()
    {
        // must be called (performance will be slightly down).
        uDesktopDuplication.Manager.primary.useGetPixels = true;

        var monitor = uddTexture.monitor;
        if (!monitor.hasBeenUpdated) return;

        if (monitor.GetPixels(colors, x, y, width, height)) {
            texture.SetPixels32(colors);
            texture.Apply();
        }

        Debug.Log(monitor.GetPixel(monitor.cursorX, monitor.cursorY));
    }
}
