using UnityEngine;
using System.Collections;
using System.Collections.Generic;

namespace uDesktopDuplication
{

public class uDD_Manager : MonoBehaviour
{
    private static uDD_Manager instance_;
    public static uDD_Manager instance 
    {
        get 
        { 
            if (instance_) return instance_;
            var go = new GameObject("uDesktopDuplicationManager");
            return go.AddComponent<uDD_Manager>();
        }
    }

    private List<uDD_Monitor> monitors_ = new List<uDD_Monitor>();
    static public List<uDD_Monitor> monitors
    {
        get { return instance.monitors_; }
    }

    static public uDD_Monitor primary
    {
        get 
        {
            return instance.monitors_.Find(monitor => monitor.isPrimary);
        }
    }

    private Coroutine renderCoroutine_ = null;

    void Awake()
    {
        if (instance_ != null) return;
        instance_ = this;

        for (int i = 0; i < uDD_Monitor.count; ++i) {
            monitors.Add(new uDD_Monitor(i));
        }
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
            foreach (var monitor in monitors) {
                if (monitor.shouldBeUpdated) {
                    monitor.Render();
                }
                monitor.shouldBeUpdated = false;
            }
        }
    }
}

}