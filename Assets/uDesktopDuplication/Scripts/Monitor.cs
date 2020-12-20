using UnityEngine;

namespace uDesktopDuplication
{

public class Monitor
{
    public Monitor(int id)
    {
        this.id = id;

        switch (state)
        {
            case DuplicatorState.Ready:
                break;
            case DuplicatorState.Running:
                break;
            case DuplicatorState.InvalidArg:
                Debug.LogErrorFormat("[uDD] {0}:{1} => Invalid.", id, name);
                break;
            case DuplicatorState.AccessDenied:
                Debug.LogWarningFormat("[uDD] {0}:{1} => Access Denied.", id, name);
                break;
            case DuplicatorState.Unsupported:
                Debug.LogWarningFormat("[uDD] {0}:{1} => Unsupported.", id, name);
                break;
            case DuplicatorState.SessionDisconnected:
                Debug.LogWarningFormat("[uDD] {0}:{1} => Disconnected.", id, name);
                break;
            case DuplicatorState.NotSet:
                Debug.LogErrorFormat("[uDD] {0}:{1} => Something wrong.", id, name);
                break;
            default:
                Debug.LogErrorFormat("[uDD] {0}:{1} => Unknown error.", id, name);
                break;
        }

        if (dpiX == 0 || dpiY == 0) {
            Debug.LogWarningFormat("[uDD] {0}:{1} => Could not get DPI", id, name);
        }
    }

    ~Monitor()
    {
    }

    public int id 
    { 
        get; 
        private set; 
    }

    public bool exists
    { 
        get { return id < Manager.monitorCount; } 
    }

    public DuplicatorState state
    {
        get { return Lib.GetState(id); }
    }

    public bool available
    {
        get 
        { 
            return 
                state == DuplicatorState.Ready || 
                state == DuplicatorState.Running; 
        }
    }

    public string name
    { 
        get { return Lib.GetName(id); }
    }

    public bool isPrimary
    { 
        get { return Lib.IsPrimary(id); }
    }

    public int left
    { 
        get { return Lib.GetLeft(id); }
    }

    public int right
    { 
        get { return Lib.GetRight(id); }
    }

    public int top
    { 
        get { return Lib.GetTop(id); }
    }

    public int bottom
    { 
        get { return Lib.GetBottom(id); }
    }

    public int width 
    { 
        get { return Lib.GetWidth(id); }
    }

    public int height
    { 
        get { return Lib.GetHeight(id); }
    }

    public int dpiX
    { 
        get 
        {
            var dpi = Lib.GetDpiX(id); 
            if (dpi == 0) dpi = 100; // when monitors are duplicated
            return dpi;
        }
    }

    public int dpiY
    { 
        get 
        {
            var dpi = Lib.GetDpiY(id); 
            if (dpi == 0) dpi = 100; // when monitors are duplicated
            return dpi;
        }
    }

    public bool isHDR
    {
        get { return Lib.IsHDR(id); }
    }

    public float widthMeter
    { 
        get { return width / dpiX * 0.0254f; }
    }

    public float heightMeter
    { 
        get { return height / dpiY * 0.0254f; }
    }

    public MonitorRotation rotation
    { 
        get { return Lib.GetRotation(id); }
    }

    public float aspect
    { 
        get { return (float)width / height; }
    }

    public bool isHorizontal
    { 
        get 
        {
            return 
                (rotation == MonitorRotation.Identity) || 
                (rotation == MonitorRotation.Rotate180);
        }
    }

    public bool isVertical 
    { 
        get 
        {
            return 
                (rotation == MonitorRotation.Rotate90) || 
                (rotation == MonitorRotation.Rotate270);
        }
    }

    public bool isCursorVisible
    { 
        get { return Lib.IsCursorVisible(); }
    }

    public int cursorX
    { 
        get { return Lib.GetCursorMonitorId() == id ? Lib.GetCursorX() : -1; }
    }

    public int cursorY
    { 
        get { return Lib.GetCursorMonitorId() == id ? Lib.GetCursorY() : -1; }
    }

    public int systemCursorX
    { 
        get 
        { 
            var p = Utility.GetCursorPos();
            return p.x - left;
        }
    }

    public int systemCursorY
    { 
        get 
        { 
            var p = Utility.GetCursorPos();
            return p.y - top;
        }
    }

    public int cursorShapeWidth
    { 
        get { return Lib.GetCursorShapeWidth(); }
    }

    public int cursorShapeHeight
    { 
        get { return Lib.GetCursorShapeHeight(); }
    }

    public CursorShapeType cursorShapeType
    { 
        get { return Lib.GetCursorShapeType(); }
    }

    public int moveRectCount
    { 
        get { return Lib.GetMoveRectCount(id); }
    }

    public DXGI_OUTDUPL_MOVE_RECT[] moveRects
    {
        get { return Lib.GetMoveRects(id); }
    }

    public int dirtyRectCount
    { 
        get { return Lib.GetDirtyRectCount(id); }
    }

    public RECT[] dirtyRects
    {
        get { return Lib.GetDirtyRects(id); }
    }

    public System.IntPtr buffer
    {
        get { return Lib.GetBuffer(id); }
    }

    public bool hasBeenUpdated
    {
        get { return Lib.HasBeenUpdated(id); }
    }

    bool useGetPixels_ = false;
    public bool useGetPixels
    {
        get
        {
            return useGetPixels_;
        }
        set
        {
            useGetPixels_ = value;
            Lib.UseGetPixels(id, value);
        }
    }

    public bool shouldBeUpdated
    {
        get; 
        set;
    }

    private static Texture2D errorTexture_;
    private static readonly string errorTexturePath = "uDesktopDuplication/Textures/NotAvailable";
    private Texture2D errorTexture
    {
        get 
        {
            return errorTexture_ ?? 
                (errorTexture_ = Resources.Load<Texture2D>(errorTexturePath));   
        }
    }

    private Texture2D texture_;
    private System.IntPtr texturePtr_;
    public Texture2D texture 
    {
        get 
        { 
            if (!available) return errorTexture;
            if (texture_ == null) CreateTextureIfNeeded();
            return texture_;
        }
    }

    public void Render()
    {
        if (texture_ && available && texturePtr_ != System.IntPtr.Zero) {
            Lib.SetTexturePtr(id, texturePtr_);
            GL.IssuePluginEvent(Lib.GetRenderEventFunc(), id);
        }
    }

    public void GetCursorTexture(System.IntPtr ptr)
    {
        Lib.GetCursorTexture(ptr);
    }

    public void CreateTextureIfNeeded()
    {
        if (!available) return;

        var w = isHorizontal ? width : height;
        var h = isHorizontal ? height : width;
        bool shouldCreate = true;

        if (texture_ && texture_.width == w && texture_.height == h) {
            shouldCreate = false;
        }

        if (w <= 0 || h <= 0) {
            shouldCreate = false;
        }

        if (shouldCreate) {
            CreateTexture();
        }
    }

    void CreateTexture()
    {
        DestroyTexture();
        var w = isHorizontal ? width : height;
        var h = isHorizontal ? height : width;
        texture_ = new Texture2D(w, h, TextureFormat.BGRA32, false);
        texturePtr_ = texture_.GetNativeTexturePtr();
    }

    public void DestroyTexture()
    {
        if (texture_) {
            Object.Destroy(texture_);
            texture_ = null;
            texturePtr_ = System.IntPtr.Zero;
        }
    }

    public void Reinitialize()
    {
        CreateTextureIfNeeded();
    }

    public Color32[] GetPixels(int x, int y, int width, int height)
    {
        if (!useGetPixels_) {
            Debug.LogErrorFormat("Please set Monitor[{0}].useGetPixels as true.", id);
            return null;
        }
        return Lib.GetPixels(id, x, y, width, height);
    }

    public bool GetPixels(Color32[] colors, int x, int y, int width, int height)
    {
        if (!useGetPixels_) {
            Debug.LogErrorFormat("Please set Monitor[{0}].useGetPixels as true.", id);
            return false;
        }
        return Lib.GetPixels(id, colors, x, y, width, height);
    }

    public Color32 GetPixel(int x, int y)
    {
        if (!useGetPixels_) {
            Debug.LogErrorFormat("Please set Monitor[{0}].useGetPixels as true.", id);
            return Color.black;
        }
        return Lib.GetPixel(id, x, y);
    }
}

}