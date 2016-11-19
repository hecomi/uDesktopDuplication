using UnityEngine;

[RequireComponent(typeof(MultipleMonitorCreator))]
public class MultipleMonitorLayouter : MonoBehaviour
{
    protected MultipleMonitorCreator creator_;
    public bool updateEveryFrame = true;
    public float margin = 0.1f;
    [Range(0f, 10f)] public float thickness = 1f;

    void Start()
    {
        creator_ = GetComponent<MultipleMonitorCreator>();
        Layout();
    }

    protected virtual void Update()
    {
        margin = Mathf.Max(margin, 0f);

        if (updateEveryFrame) {
            Layout();
        }
    }

    protected virtual void Layout()
    {
        var monitors = creator_.monitors;
        var n = monitors.Count;

        var totalWidth = 0f;
        foreach (var info in monitors) {
            var width = info.gameObject.transform.localScale.x * (info.mesh.bounds.extents.x * 2f);
            totalWidth += width;
        }
        totalWidth += margin * (n - 1);

        var x = -totalWidth / 2;

        foreach (var info in monitors) {
            var width = info.gameObject.transform.localScale.x;
            x += (width * info.mesh.bounds.extents.x);
            info.gameObject.transform.localPosition = new Vector3(x, 0f, 0f);
            info.gameObject.transform.localRotation = info.originalRotation;
            x += (width * info.mesh.bounds.extents.x) + margin;
        }

        foreach (var info in monitors) {
            info.uddTexture.thickness = thickness;
        }
    }

}

