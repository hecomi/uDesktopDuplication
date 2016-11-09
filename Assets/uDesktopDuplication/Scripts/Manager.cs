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
            if (instance_) {
                return instance_;
            }

            var manager = FindObjectOfType<Manager>();
            if (manager) {
                manager.Awake();
                return manager;
            }

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

    static public int cursorMonitorId 
    {
        get { return Lib.GetCursorMonitorId(); }
    }

    static public Monitor primary
    {
        get 
        {
            return instance.monitors_.Find(monitor => monitor.isPrimary);
        }
    }

    [SerializeField] int desktopDuplicationApiTimeout = 0;
    [SerializeField] float retryReinitializationDuration = 1f;

    private Coroutine renderCoroutine_ = null;
    private bool shouldReinitialize = false;
    private float reinitializationTimer = 0f;

    public delegate void ReinitializeHandler();
    public static event ReinitializeHandler onReinitialized;

    private Lib.DebugLogDelegate logFunc = msg => Debug.Log(msg);
    private Lib.DebugLogDelegate errorFunc = msg => Debug.LogError(msg);

    void Awake()
    {
        Lib.InitializeUDD();

        if (instance_ != null) return;
        instance_ = this;

        CreateMonitors();

        Lib.SetTimeout(desktopDuplicationApiTimeout);
    }

    void OnApplicationQuit()
    {
        Lib.FinalizeUDD();
    }

    void OnEnable()
    {
        renderCoroutine_ = StartCoroutine(OnRender());

        Lib.SetLogFunc(logFunc);
        Lib.SetErrorFunc(errorFunc);
    }

    void OnDisable()
    {
        Lib.SetLogFunc(null);
        Lib.SetErrorFunc(null);

        if (renderCoroutine_ != null) {
            StopCoroutine(renderCoroutine_);
            renderCoroutine_ = null;
        }
    }

    void Update()
    {
        Lib.Update();
        ReinitializeIfNeeded();
        UpdateMessage();
    }

    [ContextMenu("Reinitialize")]
    void Reinitialize()
    {
        Debug.Log("[uDesktopDuplication] Reinitialize");
        Lib.Reinitialize();
        if (onReinitialized != null) onReinitialized();
    }

    void ReinitializeIfNeeded()
    {
        for (int i = 0; i < monitors.Count; ++i) {
            var monitor = monitors[i];
            if (monitor.state == MonitorState.NotSet ||
                monitor.state == MonitorState.AccessLost || 
                monitor.state == MonitorState.AccessDenied ||
                monitor.state == MonitorState.SessionDisconnected) {
                if (!shouldReinitialize) {
                    shouldReinitialize = true;
                    reinitializationTimer = 0f;
                    break;
                }
            }
        }

        if (shouldReinitialize) {
            if (reinitializationTimer > retryReinitializationDuration) {
                Reinitialize();
                shouldReinitialize = false;
            }
            reinitializationTimer += Time.deltaTime;
        }
    }

    void UpdateMessage()
    {
        var message = Lib.PopMessage();
        while (message != Message.None) {
            switch (message) {
                case Message.Reinitialized:
                    ReinitializeMonitors();
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
        monitors.Clear();
        for (int i = 0; i < monitorCount; ++i) {
            monitors.Add(new Monitor(i));
        }
    }

    void ReinitializeMonitors()
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