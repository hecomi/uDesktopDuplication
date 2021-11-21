using UnityEngine;
using UnityEngine.Assertions;

[RequireComponent(typeof(uDesktopDuplication.Texture))]
public class DisplacementMapping : MonoBehaviour
{
    public enum TargetMonitor
    { 
        Self,
        Next,
        Prev,
    }
    public TargetMonitor target = TargetMonitor.Self;

    uDesktopDuplication.Texture uddTexture_;

    int dispTexId_;
    int dispFactorId_;
    int tessMinDistId_;
    int tessMaxDistId_;
    int tessFactorId_;

    [Range(0.0f, 10f)] public float displacementFactor = 0.1f;
    [Range(0.1f, 10f)] public float tessellationMinDist = 0.5f;
    [Range(1.0f, 50f)] public float tessellationMaxDist = 25f;
    [Range(1.0f, 50f)] public float tessellationFactor = 25f;

    void Start()
    {
        uddTexture_ = GetComponent<uDesktopDuplication.Texture>();
        Assert.IsNotNull(uddTexture_);

        dispTexId_ = Shader.PropertyToID("_DispTex");
        dispFactorId_ = Shader.PropertyToID("_DispFactor");
        tessMinDistId_ = Shader.PropertyToID("_TessMinDist");
        tessMaxDistId_ = Shader.PropertyToID("_TessMaxDist");
        tessFactorId_ = Shader.PropertyToID("_TessFactor");
    }

    void Update()
    {
        var id = uddTexture_.monitor.id;
        switch (target) {
            case TargetMonitor.Self: break;
            case TargetMonitor.Next: ++id; break;
            case TargetMonitor.Prev: --id; break;
        }
        id = Mathf.Clamp(id, 0, uDesktopDuplication.Manager.monitorCount - 1);
        var monitor = uDesktopDuplication.Manager.GetMonitor(id);
        monitor.shouldBeUpdated = true;

        uddTexture_.material.SetTexture(dispTexId_, monitor.texture);
        uddTexture_.material.SetFloat(dispFactorId_, displacementFactor);
        uddTexture_.material.SetFloat(tessMinDistId_, tessellationMinDist);
        uddTexture_.material.SetFloat(tessMaxDistId_, tessellationMaxDist);
        uddTexture_.material.SetFloat(tessFactorId_, tessellationFactor);
    }
}