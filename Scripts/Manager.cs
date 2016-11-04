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
        get { return Lib.GetMonitorCount(); }
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

    private Coroutine renderCoroutine_ = null;

    void Awake()
    {
        Lib.InitializeUDD();

        if (instance_ != null) return;
        instance_ = this;

        CreateMonitors();

        Lib.SetTimeout(timeout);
    }

    void OnApplicationQuit()
    {
        Lib.FinalizeUDD();
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

    void Update()
    {
        Lib.Update();

        var message = Lib.PopMessage();
        while (message != Message.None) {
            switch (message) {
                case Message.Reinitialized:
                    Debug.Log("Reinitialize");
                    Reinitialize();
                    break;
                default:
                    break;
            }
            message = Lib.PopMessage();
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
        }
    }

    void CreateMonitors()
    {
        for (int i = 0; i < monitorCount; ++i) {
            monitors.Add(new Monitor(i));
        }
    }

    void Reinitialize()
    {
        for (int i = 0; i < monitorCount; ++i) {
            if (i == monitors.Count) {
                monitors.Add(new Monitor(i));
            }
            monitors[i].Reinitialize();
        }
    }
}

}