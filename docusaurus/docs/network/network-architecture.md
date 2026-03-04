---
sidebar_position: 2
---

# Network Architecture

Multiplayer architecture with UDP networking, entity synchronization, and client-server model.

## Overview

**Key Features:**
-  UDP-based networking (Boost.Asio)
-  Client-server architecture with server authority
-  Entity synchronization system (configurable 60-120 Hz)
-  Binary protocol (see [RFC](protocol.md))
-  Connection management
-  Packet serialization

**Components:**
- `engine/src/Systems/NetworkSystem/` - Engine networking
- `server/src/systems/NetworkPacketSystem/` - Server logic
- `client/src/systems/NetworkPacketSystem/` - Client logic

---

## Architecture

```
┌─────────────┐                            ┌─────────────┐
│   Client    │                            │   Server    │
│             │        UDP Packets         │             │
│ NetworkSys  │◄──────────────────────────►│ NetworkSys  │
│             │                            │             │
│ PacketSys   │   Player Input ────────►   │ PacketSys   │
│             │   ◄──────── Entity State   │             │
└─────────────┘                            └─────────────┘
      │                                            │
      ▼                                            ▼
 Game State                                  Game State
 (Rendering)                                 (Authoritative)
```

### Manager vs System Separation

- **NetworkManager**: Low-level UDP socket, send/receive
- **NetworkSystem**: ECS integration, event handling

---

## Entity Synchronization

The server sends entity state updates to all clients at a configurable rate.

### EntitySynchronizerSystem (Server)

```cpp
class EntitySynchronizerSystem : public ISystem {
    float syncRate = 60.0F;  // 60 updates per second (configurable)
    float timer = 0.0F;

    void update(EcsManager& ecs, float deltaTime) override {
        timer += deltaTime;

        if (timer >= (1.0F / syncRate)) {
            // Gather all game entities
            // Serialize positions, velocities, healths, etc.
            // Send UPDATE packet to all clients
            timer = 0.0F;
        }
    }
};
```

**Sync Rate Options:**
- 60 Hz: Standard (every ~16ms)
- 120 Hz: High performance (every ~8ms)
- 30 Hz: Low bandwidth (every ~33ms)

**Configurable in:** `server/src/main.cpp`

---

## Network Events

### StartNetworkEvent
Start client connection:

```cpp
struct StartNetworkEvent {
    std::uint16_t port;
    std::string serverAddress;
};

// Client connects to server
StartNetworkEvent connect(57208, "127.0.0.1");
ecs->publishEvent(connect);
```

### SendPunctualPacketEvent
Send a packet once:

```cpp
struct SendPunctualPacketEvent {
    int recipientId;  // 1 = broadcast to all
    std::vector<std::uint8_t> data;
};

// Send CONNECT packet
SendPunctualPacketEvent sendConnect(1, connectData);
ecs->publishEvent(sendConnect);
```

---

## Packet Types

See [Protocol RFC](protocol.md) for complete packet specification.

**Common Packets:**

| Packet | Direction | Purpose |
|--------|-----------|---------|
| `CONNECT` | Client → Server | Request connection |
| `DISCONNECT` | Both | Close connection |
| `INPUT` | Client → Server | Player input state |
| `UPDATE` | Server → Client | Entity state sync |
| `SPAWN` | Server → Client | New entity created |
| `DESTROY` | Server → Client | Entity destroyed |

---

## Client-Server Model

### Server Authority

The server is **authoritative** for all game state:
- Enemy spawning
- Collision detection
- Damage calculation
- Score updates
- Entity destruction

**Client never**:
- Creates enemies
- Determines hits
- Awards points
- Destroys entities (except via input)

### Client Responsibility

Clients:
- Render game state
- Send input to server
- Apply state updates from server
- Provide visual/audio feedback

---

## Connection Management

### Client Connection

```cpp
// 1. Start network
StartNetworkEvent start(57208, "127.0.0.1");
ecs->publishEvent(start);

// 2. Send CONNECT packet
struct EmptyPayload {};
auto connectData = Serializer::serializePacket(
    PacketType::CONNECT,
    EmptyPayload{}
);

SendPunctualPacketEvent send(1, connectData);
ecs->publishEvent(send);

// 3. Wait for server response (assigned player ID)
```

### Server Connection Handling

```cpp
// NetworkPacketSystem on server
void handleConnect(ClientId clientId) {
    // 1. Create player entity
    EntityId player = ecs.createEntity();
    ecs.addComponent(player, PlayerComponent{clientId});

    // 2. Send player ID to client
    // 3. Notify other clients of new player
    // 4. Start sending updates to new client
}
```

### Disconnection

```cpp
// Client disconnect
auto disconnectData = Serializer::serializePacket(
    PacketType::DISCONNECT,
    EmptyPayload{}
);

SendPunctualPacketEvent send(1, disconnectData);
ecs->publishEvent(send);
```

---

## Bandwidth Optimization

### What's Sent

**Every Frame (60 Hz):**
- Entity positions
- Entity velocities
- Player health/lives

**On Event:**
- Entity spawn/destroy
- Collision events
- Score updates

### What's NOT Sent

- Static data (sprites, textures)
- Client-side effects (particles)
- UI state
- Sound effects (triggered locally)

**Typical Bandwidth:**
- 10 players, 50 entities: ~10-20 KB/s per client
- Boss fight (100 entities): ~30-50 KB/s per client

---

## Example: Complete Network Setup

### Server

```cpp
int main() {
    auto ecs = std::make_unique<EcsManager>();

    // Register network systems
    ecs->registerSystem<NetworkSystem>(*ecs);
    ecs->registerSystem<NetworkPacketSystem>(*ecs);
    ecs->registerSystem<EntitySynchronizerSystem>(*ecs);

    // Start server
    GameEngine::StartNetworkEvent startServer(57208, "0.0.0.0");
    ecs->publishEvent(startServer);

    std::cout << "Server listening on port 57208\n";

    // Game loop
    while (running) {
        ecs->update(deltaTime);
        // Automatically syncs entities to clients
    }
}
```

### Client

```cpp
int main() {
    auto ecs = std::make_unique<EcsManager>();

    // Register network systems
    ecs->registerSystem<NetworkSystem>(*ecs);
    ecs->registerSystem<NetworkPacketSystem>(*ecs);
    ecs->registerSystem<InputMappingSystem>(*ecs);

    // Connect to server
    GameEngine::StartNetworkEvent connect(57208, "127.0.0.1");
    ecs->publishEvent(connect);

    // Send CONNECT packet
    auto connectData = Serializer::serializePacket(
        PacketType::CONNECT, EmptyPayload{}
    );
    SendPunctualPacketEvent send(1, connectData);
    ecs->publishEvent(send);

    // Game loop
    while (running) {
        ecs->update(deltaTime);
        // Receives and applies server updates
    }
}
```

---

## Performance Tuning

### Sync Rate

```cpp
// High performance (120 Hz)
entitySync.setSyncRate(120.0F);

// Standard (60 Hz)
entitySync.setSyncRate(60.0F);

// Low bandwidth (30 Hz)
entitySync.setSyncRate(30.0F);
```

**Trade-off:** Higher rate = smoother, more bandwidth

### Packet Compression

Future enhancement - currently uncompressed binary.

---

## Troubleshooting

### Client Can't Connect
- Check server is running
- Verify IP address and port
- Check firewall settings
- Ensure server called `StartNetworkEvent`

### Laggy Movement
- Reduce sync rate on server
- Check network latency (ping)
- Implement client prediction (future)

### Entities Not Spawning
- Verify server authority (only server spawns)
- Check EntitySynchronizerSystem is running
- Verify SPAWN packets sent

---

## Related Documentation

- [Protocol RFC](protocol.md) - Packet format specification
- [Engine Architecture](../engine/architecture.md) - Event system
- [Input System](../engine/input-system.md) - Input to network packets

---

**Last Updated:** November 2025

**Refactor:** RTP-83 (Manager/System separation)
