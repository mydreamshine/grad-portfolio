using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;

public class AABBDownloader : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        StreamWriter writer_;
        string strFilePath = ".\\map.txt";
        writer_ = File.CreateText(strFilePath);

        int child_num = gameObject.transform.childCount;
        writer_.Write(child_num + " ");
        for (int i=0;i<child_num;++i)
        {
            Transform child = gameObject.transform.GetChild(i);
            Renderer childsRenderer = child.GetComponent<Renderer>();
            Debug.Log("바운딩 박스 " + child.name + " : " + childsRenderer.bounds.center + ", " + childsRenderer.bounds.extents);
            writer_.Write(childsRenderer.bounds.center.x + " ");
            writer_.Write(childsRenderer.bounds.center.y + " ");
            writer_.Write(childsRenderer.bounds.center.z + " ");
            writer_.Write(childsRenderer.bounds.extents.x + " ");
            writer_.Write(childsRenderer.bounds.extents.y + " ");
            writer_.Write(childsRenderer.bounds.extents.z + " ");
        }
        writer_.Close();
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
