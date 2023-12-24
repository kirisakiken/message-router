# Message Router

Message Router is a service which has capabilities such as accepting client connections,
listening messages from clients, sending messages to clients.

## Build & Test

- `~/message-router$ cmake . -B build`
- `~/message-router$ cd build`
- `~/message-router/build$ make`
- `~/message-router/build$ ./message_router`
- Open up another terminals that will work as clients
- `~/message-router$ cd examples`
- `~/message-router/examples$ python3 client.py`
- Enter messages from client terminals

## Behavours;
1. `OnClientConnect`: send welcome packet to client
2. `OnClientDisconect`: notify other clients
3. `OnClientMessageReceived`: broadcast received message to other clients

---

### Usage examples;
Example message structure (processor server side implementation);

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

ROADMAP;
1. OOP structure and SOLID refactor. (optional until 4)
2. Dedicated processor server. Meaning that this router may be used as transaction gateway.
3. After 2, implementation of message rules. Meaning that processor server may dictate which message(s) should be sent to which client(s).
4. UDP implementation.
5. After 4, multi thread implementation to run TCP/UDP connections at the same time.

CHECKLIST (MESSAGE STRUCTURE);
- header examples
