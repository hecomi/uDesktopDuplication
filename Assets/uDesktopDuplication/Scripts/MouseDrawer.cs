using UnityEngine;

namespace uDesktopDuplication
{

[RequireComponent(typeof(uDesktopDuplication))]
public class MouseDrawer : MonoBehaviour
{
    [SerializeField]
    Transform cursor;
    [SerializeField]
    Vector2 modelScale = Vector2.one;
    [SerializeField]
    Vector2 offset = new Vector2(0.5f, 0.5f);

    uDesktopDuplication udd_;

    void OnEnable()
    {
        udd_ = GetComponent<uDesktopDuplication>();
        udd_.onMouseMove += OnMouseMove;
    }

    void OnDisable()
    {
        udd_.onMouseMove -= OnMouseMove;
    }

    void OnMouseMove(Vector2 pos)
    {
        var x = pos.x * modelScale.x * 0.5f;
        var y = pos.y * modelScale.y * 0.5f;
        var iy = udd_.invertY ? +1 : -1;
        var localPos = transform.right * x + iy * transform.up * y;

        var worldPos = transform.TransformPoint(localPos);
        worldPos += cursor.right * offset.x * cursor.localScale.x;
        worldPos += -cursor.up * offset.y * cursor.localScale.y;

        cursor.position = worldPos;
    }
}

}