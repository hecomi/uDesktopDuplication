#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>
#include <string>

struct IUnityInterfaces;

struct Pointer
{
    bool   isVisible        = false;
    int    x                = -1;
    int    y                = -1;
    BYTE*  apiBuffer        = nullptr;
    UINT   apiBufferSize    = 0;
    BYTE*  bgra32Buffer     = nullptr;
    UINT   bgra32BufferSize = 0;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO shapeInfo;
};

struct Monitor
{
    int                     id            = -1;
    IDXGIOutputDuplication* deskDupl      = nullptr;
    ID3D11Texture2D*        texture       = nullptr;
    DXGI_OUTPUT_DESC        outputDesc;
    MONITORINFOEX           monitorInfo;
    Pointer pointer;
};

class Duplication
{
public:
    explicit Duplication(IUnityInterfaces* unity);
    ~Duplication();
    void OnRender(int id);

public:
    void UpdatePointerTexture(int id, ID3D11Texture2D* ptr);
    void SetTimeout(int timeout);
    void SetTexturePtr(int id, void* texture);

public:
    int GetMonitorCount() const;
    void GetName(int id, char* buf, int len) const;
    bool IsPrimary(int id) const;
    int GetWidth(int id) const;
    int GetHeight(int id) const;
    int IsPointerVisible(int id) const;
    int GetPointerX(int id) const;
    int GetPointerY(int id) const;
    int GetPointerShapeWidth(int id) const;
    int GetPointerShapeHeight(int id) const;
    int GetPointerShapePitch(int id) const;
    int GetPointerShapeType(int id) const;
    int GetErrorCode() const;
    void GetErrorMessage(char* buf, int len) const;

private:
    void Initialize();
    void Finalize();
    bool UpdateMouse(const DXGI_OUTDUPL_FRAME_INFO& frameInfo, Monitor& monitor);

    bool IsValidId(int id) const;

    IUnityInterfaces*    unity_        = nullptr;
    ID3D11Device*        device_       = nullptr;
    int                  mouseMonitor_ = 0;
    int                  timeout_      = 10;
    HRESULT              errorCode_    = 0;
    std::string          errorMessage_ = "";
    std::vector<Monitor> monitors_;
};