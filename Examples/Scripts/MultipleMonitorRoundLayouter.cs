using UnityEngine;

public class MultipleMonitorRoundLayouter : MultipleMonitorLayouter
{
    [SerializeField] float radius = 10f;
    [SerializeField] float offsetAngle = 0f;

    void OnDisable()
    {
        foreach (var info in creator_.monitors) {
            info.uddTexture.bend = uDesktopDuplication.Texture.Bend.Off;
        }
    }

    protected override void Layout()
    {
        var monitors = creator_.monitors;
        var n = monitors.Count;

        var totalWidth = 0f;
        foreach (var info in monitors) {
            var width = info.uddTexture.monitor.widthMeter * (info.mesh.bounds.extents.x * 2f);
            totalWidth += width;
        }
        totalWidth += margin * (n - 1);

        radius = Mathf.Max(radius, (totalWidth + margin) / (2 * Mathf.PI));
        var totalAngle = totalWidth / radius;

        float angle = -totalAngle / 2 + offsetAngle * Mathf.Deg2Rad;
        foreach (var info in monitors) {
            var uddTex = info.uddTexture;
            var width = uddTex.monitor.widthMeter * (info.mesh.bounds.extents.x * 2f);

            angle += (width / radius) * 0.5f;
            uddTex.transform.localPosition = radius * new Vector3(Mathf.Sin(angle), 0f, Mathf.Cos(angle) - 1f);
            uddTex.transform.localRotation = Quaternion.AngleAxis(angle * Mathf.Rad2Deg, Vector3.up) * info.originalRotation;
            angle += (width * 0.5f + margin) / radius;

            if (creator_.meshForwardDirection == MultipleMonitorCreator.MeshForwardDirection.Y) {
                uddTex.bend = uDesktopDuplication.Texture.Bend.Y;
            } else {
                uddTex.bend = uDesktopDuplication.Texture.Bend.Z;
            }
            uddTex.radius = radius;
            uddTex.width = uddTex.transform.localScale.x;
        }
    }

    protected override void Update()
    {
        base.Update();

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

