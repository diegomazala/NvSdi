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

    public KeyboardEvent[] Events;


    void Update()
    {
        for (int i = 0; i < Events.Length; ++i)
        {
            Events[i].InvokeIfTriggered();
        }
    }

}
