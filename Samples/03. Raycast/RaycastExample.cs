using UnityEngine;

public class RaycastExample : MonoBehaviour
{
    public Transform from;
    public Transform to;

    void Update()
    {
        if (!from || !to) return;

        Debug.DrawLine(from.position, to.position, Color.red);

        foreach (var uddTexture in GameObject.FindObjectsOfType<uDesktopDuplication.Texture>()) {
            var result = uddTexture.RayCast(from.position, to.position - from.position);
            if (result.hit) {
                Debug.DrawLine(result.position, result.position + result.normal, Color.yellow);
                Debug.Log("COORD: " + result.coords + ", DESKTOP: " + result.desktopCoord);
            }
        }
    }
}
