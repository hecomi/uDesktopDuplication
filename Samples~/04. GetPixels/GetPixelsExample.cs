using UnityEngine;

public class GetPixelsExample : MonoBehaviour
{
    [SerializeField] uDesktopDuplication.Texture uddTexture;

    [SerializeField] int x = 100;
    [SerializeField] int y = 100;
    [SerializeField] int w = 64;
    [SerializeField] int h = 32;
    
    public Texture2D texture;
    Color32[] colors;

    void CreateTextureIfNeeded()
    {
        if (!texture || texture.width != w || texture.height != h)
        {
            colors = new Color32[w * h];
            texture = new Texture2D(w, h, TextureFormat.ARGB32, false);
            GetComponent<Renderer>().material.mainTexture = texture;
        }
    }

    void Start()
    {
        CreateTextureIfNeeded();
    }

    void Update()
    {
        CreateTextureIfNeeded();

        // must be called (performance will be slightly down).
        uDesktopDuplication.Manager.primary.useGetPixels = true;

        var monitor = uddTexture.monitor;
        if (!monitor.hasBeenUpdated) return;

        if (monitor.GetPixels(colors, x, y, w, h)) {
            texture.SetPixels32(colors);
            texture.Apply();
        }

        Debug.Log(monitor.GetPixel(monitor.cursorX, monitor.cursorY));
    }
}
