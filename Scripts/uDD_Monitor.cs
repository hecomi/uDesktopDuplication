using UnityEngine;

namespace uDesktopDuplication
{

public class uDD_Monitor
{
    public uDD_Monitor(int id)
    {
        this.id = id;
    }

    public static int count
    {
        get { return uDD_Lib.GetMonitorCount(); }
    }

    public int id 
    { 
        get; 
        private set; 
    }

    public string name
    { 
        get { return uDD_Lib.GetName(id); }
    }

    public bool isPrimary
    { 
        get { return uDD_Lib.IsPrimary(id); }
    }

    public int width 
    { 
        get { return uDD_Lib.GetWidth(id); }
    }

    public int height
    { 
        get { return uDD_Lib.GetHeight(id); }
    }

    public bool isPointerVisible
    { 
        get { return uDD_Lib.IsPointerVisible(id); }
    }

    public int pointerX
    { 
        get { return uDD_Lib.GetPointerX(id); }
    }

    public int pointerY
    { 
        get { return uDD_Lib.GetPointerY(id); }
    }

    public bool shouldBeUpdated
    {
        get; 
        set;
    }

    private Texture2D texture_;
    public Texture2D texture 
    {
        get 
        { 
            if (texture_ == null) {
                texture_ = new Texture2D(width, height, TextureFormat.BGRA32, false);
                uDD_Lib.SetTexturePtr(id, texture_.GetNativeTexturePtr());
            }
            return texture_;
        }
    }

    public void Render()
    {
        GL.IssuePluginEvent(uDD_Lib.GetRenderEventFunc(), id);
    }
}

}