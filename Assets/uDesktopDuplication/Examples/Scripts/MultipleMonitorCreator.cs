using UnityEngine;

public class MultipleMonitorCreator : MonoBehaviour
{
    [SerializeField] GameObject monitorPrefab;
    [SerializeField] float scale = 10f;
    [SerializeField] float margin = 1f;
    [SerializeField] float ratio = 0.001f;

    void Start()
    {
        var monitors = uDesktopDuplication.Manager.monitors;
        var n = monitors.Count;

        var totalWidth = 0f;
        for (int i = 0 ; i < n; ++i) {
            var go = Instantiate(monitorPrefab);
            go.name = "Monitor " + i;
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            texture.monitorId = i;
            go.transform.localScale = new Vector3(texture.monitor.width * ratio, 1f, texture.monitor.height * ratio);
            go.transform.SetParent(transform);
            totalWidth += texture.monitor.width * ratio * scale;
        }

        totalWidth += margin * (n - 1);
        var x = -totalWidth / 2;
        for (int i = 0 ; i < n; ++i) {
            var go = transform.FindChild("Monitor " + i);
            var texture = go.GetComponent<uDesktopDuplication.Texture>();
            var halfWidth = texture.monitor.width * ratio * scale / 2;
            x += halfWidth;
            go.transform.localPosition = new Vector3(x, 0f, 0f);
            x += halfWidth + margin;
        }
    }
}

