#include <d3d11.h>
#include <dxgi1_2.h>
#include <memory>

class Cursor;

enum class MonitorState
{
	NotSet = -1,
	Available = 0,
	InvalidArg = 1,
	AccessDenied = 2,
	Unsupported = 3,
	CurrentlyNotAvailable = 4,
	SessionDisconnected = 5,
	AccessLost = 6,
};

class Monitor
{
public:
	using State = MonitorState;

    explicit Monitor(int id);
    ~Monitor();
    HRESULT Initialize(IDXGIOutput* output);
    HRESULT Render(UINT timeout = 0);
    void UpdateCursorTexture(ID3D11Texture2D* texture);

public:
    int GetId() const;
	State GetState() const;
    void SetUnityTexture(ID3D11Texture2D* texture);
    ID3D11Texture2D* GetUnityTexture() const;
    void GetName(char* buf, int len) const;
    bool IsPrimary() const;
	int GetLeft() const;
	int GetRight() const;
	int GetTop() const;
	int GetBottom() const;
    int GetWidth() const;
    int GetHeight() const;
	int GetRotation() const;
    IDXGIOutputDuplication* GetDeskDupl();
    const std::unique_ptr<Cursor>& GetCursor();

private:
    int id_ = -1;
	State state_ = State::NotSet;
    std::unique_ptr<Cursor> cursor_;
    IDXGIOutputDuplication* deskDupl_ = nullptr;
    ID3D11Texture2D* unityTexture_ = nullptr;
    DXGI_OUTPUT_DESC outputDesc_;
    MONITORINFOEX monitorInfo_;
};