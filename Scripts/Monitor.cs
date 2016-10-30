using UnityEngine;

namespace uDesktopDuplication
{

public class Monitor
{
    public Monitor(int id)
    {
        this.id = id;
    }

    public int id 
    { 
        get; 
        private set; 
    }

    public string name
    { 
        get { return Lib.GetName(id); }
    }

    public bool isPrimary
    { 
        get { return Lib.IsPrimary(id); }
    }

    public int width 
    { 
        get { return Lib.GetWidth(id); }
    }

    public int height
    { 
        get { return Lib.GetHeight(id); }
    }

    public float aspect
    { 
        get { return (float)width / height; }
    }

    public bool isPointerVisible
    { 
        get { return Lib.IsPointerVisible(id); }
    }

    public int pointerX
    { 
        get { return Lib.GetPointerX(id); }
    }

    public int pointerY
    { 
        get { return Lib.GetPointerY(id); }
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
                Lib.SetTexturePtr(id, texture_.GetNativeTexturePtr());
            }
            return texture_;
        }
    }

    public void Render()
    {
        GL.IssuePluginEvent(Lib.GetRenderEventFunc(), id);
    }
}

}