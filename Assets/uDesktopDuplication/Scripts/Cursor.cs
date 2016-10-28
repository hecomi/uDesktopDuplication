using UnityEngine;

namespace uDesktopDuplication
{

[AddComponentMenu("uDesktopDuplication/Cursor"), 
 RequireComponent(typeof(Texture))]
public class Cursor : MonoBehaviour
{
    [SerializeField]
    Transform cursor;
    [SerializeField]
    Vector2 modelScale = Vector2.one;
    [SerializeField]
    Vector2 offset = new Vector2(0.5f, 0.5f);

    private Texture texture_;

    void OnEnable()
    {
        texture_ = GetComponent<Texture>();
    }

    void Update()
    {
        var monitor = texture_.monitor;
        if (monitor.isPointerVisible) {
            var x = (1f * monitor.pointerX / monitor.width - 0.5f) * modelScale.x;
            var y = (0.5f - 1f * monitor.pointerY / monitor.height) * modelScale.y;
            var iy = texture_.invertY ? +1 : -1;
            var localPos = transform.right * x + iy * transform.up * y;
            var worldPos = transform.TransformPoint(localPos);
            worldPos += cursor.right * offset.x * cursor.localScale.x;
            worldPos += -cursor.up * offset.y * cursor.localScale.y;
            cursor.position = worldPos;
        }
    }
}

}