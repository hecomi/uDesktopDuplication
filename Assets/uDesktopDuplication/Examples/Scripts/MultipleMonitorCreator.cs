using UnityEngine;

public class MultipleMonitorCreator : MonoBehaviour
{
    [SerializeField] GameObject monitorPrefab;
    [SerializeField] Vector3 margin = new Vector3(20f, 0f, 0f);

    void Start()
    {
        var monitors = uDesktopDuplication.Manager.monitors;
        var n = monitors.Count;
        for (int i = 0 ; i < n; ++i) {
            var go = Instantiate(monitorPrefab);
            go.transform.position = transform.position + margin * (i - n / 2f + 0.5f);
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;
        }
    }
}

