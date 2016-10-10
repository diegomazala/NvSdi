using UnityEngine;
using UnityEngine.Events;
using System.Collections;

public class KeyboardEventMap : MonoBehaviour
{
    [System.Serializable]
    public class KeyboardEvent
    {
        public KeyCode Key = KeyCode.None;
        public UnityEvent Event = null;

        public void InvokeIfTriggered()
        {
            if (Input.GetKeyDown(Key) && Event != null)
                Event.Invoke();
        }
    }
    public bool useCtrlKey = true;
    public KeyboardEvent[] Events;


    void Update()
    {
        if (useCtrlKey)
        {
            if (!Input.GetKey(KeyCode.LeftControl))
                return;
        }

        for (int i = 0; i < Events.Length; ++i)
        {
            Events[i].InvokeIfTriggered();
        }
    }

}
