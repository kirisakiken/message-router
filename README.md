# message-router

Example message structure;

```json5
{
  code: 12,
  body: {
    // ...
    transform: {
      position: {
        x: 1,
        y: 2,
        z: 3,
      },
      rotation: {
        x: 0,
        y: 0,
        z: 0,
        w: 1,
      },
      scale: {
        x: 1,
        y: 1,
        z: 1,
      }
    }
  }
}
```

code will be used to determine how to process incoming
message body in the receiver(client) side.

E.g;
- CODE 12 may stand for `UpdatePlayerTransform`
- CODE 1 may stand for `TransformTick` where client will update transform of all object in the backend.

```json5
{
  code: 1,
  body: {
    players: [
      {
        id: "1",
        transform: {}, //...
      },
      {
        id: "2",
        transform: {}, //...
      },
    ],
    otherObjects: [
      {
        id: "3",
        transform: {}, //...
      }
    ]
  }
}
```

CHECKLIST (MESSAGE STRUCTURE);
- header to be implemented