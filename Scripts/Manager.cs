using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace uDesktopDuplication
{

public class Manager : MonoBehaviour
{
    private static Manager instance_;
    public static Manager instance 
    {
        get 
        { 
            if (instance_) return instance_;
            var go = new GameObject("uDesktopDuplicationManager");
            return go.AddComponent<Manager>();
        }
    }

    private List<Monitor> monitors_ = new List<Monitor>();
    static public List<Monitor> monitors
    {
        get { return instance.monitors_; }
    }

    static public int monitorCount
    {
        get { return instance.monitors_.Count; }
    }

    static public Monitor primary
    {
        get 
        {
            return instance.monitors_.Find(monitor => monitor.isPrimary);
        }
    }

    [SerializeField, Tooltip("Set Desktop Duplication API timeout (milliseconds).")] 
    int timeout = 0;

    [SerializeField, Tooltip("Output logs given by the plugin.")] 
    bool outputDebugLog = false;

    private Coroutine renderCoroutine_ = null;

    void Awake()
    {
        if (instance_ != null) return;
        instance_ = this;

        for (int i = 0; i < Lib.GetMonitorCount(); ++i) {
            monitors.Add(new Monitor(i));
        }

        Lib.SetTimeout(timeout);
    }

    void OnEnable()
    {
        renderCoroutine_ = StartCoroutine(OnRender());
    }

    void OnDisable()
    {
        if (renderCoroutine_ != null) {
            StopCoroutine(renderCoroutine_);
            renderCoroutine_ = null;
        }
    }

    IEnumerator OnRender()
    {
        for (;;) {
            yield return new WaitForEndOfFrame();
            for (int i = 0; i < monitors.Count; ++i) {
                var monitor = monitors[i];
                if (monitor.shouldBeUpdated) {
                    monitor.Render();
                }
                monitor.shouldBeUpdated = false;
            }
            if (outputDebugLog && Lib.GetErrorCode() != 0)
            {
                Debug.Log(Lib.GetErrorMessage());
            }
        }
    }
}

}