using UnityEngine;
using UnityEngine.SceneManagement;

public class UddSceneManager : MonoBehaviour
{
    public static UddSceneManager instance { get; set; }
    [SerializeField] string[] scenes;
    [SerializeField] int sceneNo = 0;

    void Awake()
    {
        if (!instance) {
            instance = this;
        } else {
            Destroy(gameObject);
        }
    }

    void Start()
    {
        DontDestroyOnLoad(gameObject);
        Load();
    }

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.RightArrow)) {
            Next();
        } else if (Input.GetKeyDown(KeyCode.LeftArrow)) {
            Prev();
        }
    }

    void Next()
    {
        sceneNo = (sceneNo + 1) % scenes.Length;
        Load();
    }

    void Prev()
    {
        sceneNo = (sceneNo - 1) % scenes.Length;
        Load();
    }

    void Load()
    {
        SceneManager.LoadScene(scenes[sceneNo]);
    }
}