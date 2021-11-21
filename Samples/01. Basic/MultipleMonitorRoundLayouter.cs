using UnityEngine;
using MeshForwardDirection = uDesktopDuplication.Texture.MeshForwardDirection;

public class MultipleMonitorRoundLayouter : MultipleMonitorLayouter
{
    [SerializeField] bool debugDraw = true;

    public float radius = 10f;
    public Vector3 offsetAngle = Vector3.zero;

    void OnDisable()
    {
        foreach (var info in creator_.monitors) {
            info.uddTexture.bend = false;
        }
    }

    protected override void Layout()
    {
        var monitors = creator_.monitors;
        var n = monitors.Count;

        // Keep the local scale z of monitors as 1 to bend them correctly.
        // And save width / height to use same values after reinitialization.
        for (int i = 0; i < n; ++i) {
            var info = monitors[i];
            var savedInfo = creator_.savedInfoList[i];
            var scale = info.gameObject.transform.localScale;
            if (creator_.meshForwardDirection == MeshForwardDirection.Y) {
                scale.y = 1f;
                savedInfo.widthScale = scale.x / info.originalLocalScale.x;
                savedInfo.heightScale = scale.z / info.originalLocalScale.z;
            } else {
                scale.z = 1f;
                savedInfo.widthScale = scale.x / info.originalLocalScale.x;
                savedInfo.heightScale = scale.y / info.originalLocalScale.y;
            }
            info.gameObject.transform.localScale = scale;
        }

        // keep thicness plus value.
        thickness = Mathf.Max(thickness, 0f);

        // calculate total width
        var totalWidth = 0f;
        foreach (var info in monitors) {
            var width = info.gameObject.transform.localScale.x * (info.mesh.bounds.extents.x * 2f);
            totalWidth += width;
        }
        totalWidth += margin * (n - 1);

        // expand radius if total width is larger than the circumference.
        radius = Mathf.Max(radius, (totalWidth + margin) / (2 * Mathf.PI));

        // total angle of monitors
        var totalAngle = totalWidth / radius;

        // layout
        float angle = -totalAngle / 2;
        var offsetRot = Quaternion.Euler(offsetAngle);
        foreach (var info in monitors) {
            var uddTex = info.uddTexture;
            var width = info.gameObject.transform.localScale.x * (info.mesh.bounds.extents.x * 2f);

            angle += (width / radius) * 0.5f;
            var pos = radius * new Vector3(Mathf.Sin(angle), 0f, Mathf.Cos(angle) - 1f);
            pos += radius * Vector3.forward;
            pos = offsetRot * pos;
            pos -= radius * Vector3.forward;
            uddTex.transform.localPosition = pos;
            uddTex.transform.localRotation = offsetRot * Quaternion.AngleAxis(angle * Mathf.Rad2Deg, Vector3.up) * info.originalRotation;
            angle += (width * 0.5f + margin) / radius;

            uddTex.bend = true;
            uddTex.meshForwardDirection = creator_.meshForwardDirection;
            uddTex.radius = radius;
            uddTex.thickness = thickness;
            uddTex.width = uddTex.transform.localScale.x;
        }
    }

    protected override void Update()
    {
        base.Update();
        if (debugDraw) DebugDraw();
    }

    void DebugDraw()
    {
        // draw the circumference in the scene view.
        var scale = transform.localScale.x;
        var center = transform.position - Vector3.forward * radius * scale;
        for (int i = 0; i < 100; ++i) {
            var a0 = 2 * Mathf.PI * i / 100;
            var a1 = 2 * Mathf.PI * (i + 1) / 100;
            var p0 = center + radius * scale * new Vector3(Mathf.Cos(a0), 0f, Mathf.Sin(a0));
            var p1 = center + radius * scale * new Vector3(Mathf.Cos(a1), 0f, Mathf.Sin(a1));
            Debug.DrawLine(p0, p1, Color.red);
        }
    }
}

