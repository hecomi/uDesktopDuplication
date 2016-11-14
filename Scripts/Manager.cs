﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace uDesktopDuplication
{

public class Manager : MonoBehaviour
{
    private static Manager instance_;
    public static Manager instance 
    {
        get { return CreateInstance(); }
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

    [SerializeField] DebugMode debugMode = DebugMode.File;
    [SerializeField] int desktopDuplicationApiTimeout = 0;
    [SerializeField] float retryReinitializationDuration = 1f;

    private Coroutine renderCoroutine_ = null;
    private bool shouldReinitialize_ = false;
    private float reinitializationTimer_ = 0f;

    public delegate void ReinitializeHandler();
    public static event ReinitializeHandler onReinitialized;

    public static Manager CreateInstance()
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

    public static Monitor GetMonitor(int id)
    {
        if (id < 0 || id >= Manager.monitors.Count) {
            Debug.LogErrorFormat("[uDD::Error] there is no monitor whose id is {0}.", id);
            return Manager.primary;
        }
        return monitors[Mathf.Clamp(id, 0, Manager.monitorCount - 1)];
    }

    void Awake()
    {
        Lib.SetDebugMode(debugMode);
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
        ReinitializeIfNeeded();
        UpdateMessage();
    }

    [ContextMenu("Reinitialize")]
    public void Reinitialize()
    {
        Debug.Log("[uDD] Reinitialize");
        Lib.Reinitialize();
        CreateMonitors();
        if (onReinitialized != null) {
            onReinitialized();
        }
    }

    void ReinitializeIfNeeded()
    {
        for (int i = 0; i < monitors.Count; ++i) {
            var monitor = monitors[i];
            if (monitor.state == MonitorState.NotSet ||
                monitor.state == MonitorState.AccessLost || 
                monitor.state == MonitorState.AccessDenied ||
                monitor.state == MonitorState.SessionDisconnected) {
                if (!shouldReinitialize_) {
                    shouldReinitialize_ = true;
                    reinitializationTimer_ = 0f;
                    break;
                }
            }
        }

        if (shouldReinitialize_) {
            if (reinitializationTimer_ > retryReinitializationDuration) {
                Reinitialize();
                shouldReinitialize_ = false;
            }
            reinitializationTimer_ += Time.deltaTime;
        }
    }

    void UpdateMessage()
    {
        var message = Lib.PopMessage();
        while (message != Message.None) {
            Debug.Log("[uDD] " + message);
            switch (message) {
                case Message.Reinitialized:
                    ReinitializeMonitors();
                    break;
                case Message.TextureSizeChanged:
                    RecreateTextures();
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

    void RecreateTextures()
    {
        for (int i = 0; i < monitorCount; ++i) {
            monitors[i].CreateTexture();
        }
    }
}

}