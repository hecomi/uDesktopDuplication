using UnityEngine;
using UnityEditor;
using System.Linq;

namespace uDesktopDuplication
{

[CustomEditor(typeof(Texture))]
public class TextureEditor : Editor
{
    Texture texture
    {
        get { return target as Texture; }
    }

    Monitor monitor
    {
        get { return texture.monitor; }
    }

    bool isAvailable
    {
        get { return monitor == null || !Application.isPlaying; }
    }

    bool basicsFolded_ = true;
    bool invertFolded_ = true;
    bool clipFolded_ = true;
    bool matFolded_ = true;

    SerializedProperty invertX_;
    SerializedProperty invertY_;
    SerializedProperty useClip_;
    SerializedProperty clipPos_;
    SerializedProperty clipScale_;

    void OnEnable()
    {
        invertX_ = serializedObject.FindProperty("invertX_");
        invertY_ = serializedObject.FindProperty("invertY_");
        useClip_ = serializedObject.FindProperty("useClip_");
        clipPos_ = serializedObject.FindProperty("clipPos");
        clipScale_ = serializedObject.FindProperty("clipScale");
    }

    public override void OnInspectorGUI()
    {
        serializedObject.Update();

        EditorGUILayout.Space();
        DrawMonitor();
        DrawInvert();
        DrawClip();
        DrawMaterial();

        serializedObject.ApplyModifiedProperties();
    }

    void Fold(string name, ref bool folded, System.Action func)
    {
        folded = Utils.Foldout(name, folded);
        if (folded) {
            ++EditorGUI.indentLevel;
            func();
            --EditorGUI.indentLevel;
        }
    }

    void DrawMonitor()
    {
        Fold("Monitor", ref basicsFolded_, () => {
            if (isAvailable) {
                EditorGUILayout.HelpBox("Monitor information is available only in runtime.", MessageType.Info);
                return;
            }
            var id = EditorGUILayout.Popup("Monitor", monitor.id, Manager.monitors.Select(x => x.name).ToArray());
            if (id != monitor.id) { texture.monitorId = id; }
            EditorGUILayout.IntField("ID", monitor.id);
            EditorGUILayout.Toggle("Is Primary", monitor.isPrimary);
            EditorGUILayout.EnumPopup("Rotation", monitor.rotation);
            EditorGUILayout.Vector2Field("Resolution", new Vector2(monitor.width, monitor.height));
            EditorGUILayout.Vector2Field("DPI", new Vector2(monitor.dpiX, monitor.dpiY));
            EditorGUILayout.Toggle("Is HDR", monitor.isHDR);
        });
    }

    void DrawInvert()
    {
        Fold("Invert", ref invertFolded_, () => {
            texture.invertX = EditorGUILayout.Toggle("Invert X", invertX_.boolValue);
            texture.invertY = EditorGUILayout.Toggle("Invert Y", invertY_.boolValue);
        });
    }

    void DrawClip()
    {
        Fold("Clip", ref clipFolded_, () => {
            texture.useClip = EditorGUILayout.Toggle("Use Clip", useClip_.boolValue);
            EditorGUILayout.PropertyField(clipPos_);
            EditorGUILayout.PropertyField(clipScale_);
        });
    }

    void DrawMaterial()
    {
        Fold("Material", ref matFolded_, () => {
            if (!Application.isPlaying) {
                EditorGUILayout.HelpBox("These parameters are applied to the shared material when not playing.", MessageType.Info);
            }
            texture.meshForwardDirection = (Texture.MeshForwardDirection)EditorGUILayout.EnumPopup("Mesh Forward Direction", texture.meshForwardDirection);
            texture.bend = EditorGUILayout.Toggle("Use Bend", texture.bend);
            texture.width = EditorGUILayout.FloatField("Bend Width", texture.width);
            texture.radius = EditorGUILayout.Slider("Bend Radius", texture.radius, texture.worldWidth / (2 * Mathf.PI), 100f);
            texture.thickness = EditorGUILayout.Slider("Thickness", texture.thickness, 0f, 30f);
            texture.culling = (Texture.Culling)EditorGUILayout.EnumPopup("Culling", texture.culling);
        });
    }
}

}