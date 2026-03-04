---
sidebar_position: 1
---

# Network Protocol

The R-Type network protocol is designed for real-time multiplayer gameplay with low latency and efficient bandwidth usage.

## Protocol Overview

### Design Principles

- **Binary Protocol**: All data is transmitted in binary format for efficiency
- **UDP-Based**: Uses UDP for real-time game state updates
- **Authoritative Server**: Server has final authority over game state
- **Stateless Datagrams**: Each packet is self-contained
- **Default Port**: UDP port **57208** (configurable in server)

### Network Architecture

```
┌─────────┐         UDP          ┌─────────┐
│ Client  │ ──────────────────>  │ Server  │
│         │                      │         │
│         │ <──────────────────  │         │
└─────────┘     Game Updates     └─────────┘
```

**Client Responsibilities:**
- Send player inputs to server
- Receive and apply game state updates
- Render the game world

**Server Responsibilities:**
- Process client inputs
- Update game simulation
- Broadcast state updates to all clients
- Handle connections/disconnections

## Packet Structure

All packets follow a common header structure:

```
┌──────────────────────────────────────┐
│ Header (4 bytes)                     │
├──────────────────────────────────────┤
│ - Packet Type (2 byte)               │
│ - Payload Length (2 byte)            │
├──────────────────────────────────────┤
│ Payload (0-1024 bytes)               │
└──────────────────────────────────────┘
```

### Packet Types

| Type | ID | Direction | Description |
|------|----|-----------| ------------|
| 0  | CONNECT           | C→S       | Request connection to server |
| 1  | CONNECT_OK        | S→C       | Acknowledge connection with Player ID |
| 2  | INPUT             | C→S       | Send player action input |
| 3  | DISCONNECT        | C→S       | Notify server of disconnection |
| 4  | DISCONNECT_OK     | S→C       | Acknowledge disconnection |
| 5  | PING              | C→S       | Send latency timestamp |
| 6  | PONG              | S→C       | Echo latency timestamp |
| 7  | ENTITY_CREATE     | S→C       | Inform client of new entity |
| 8  | ENTITY_UPDATE     | S→C       | Update entity position/velocity/health |
| 9  | ENTITY_DESTROY    | S→C       | Notify client of entity removal |
| 10 | TRANSFORM_UPDATE  | S→C       | Update entity transform (position/rotation) |
| 11 | PLAYER_UPDATE     | S→C       | Update player-specific state (health/score/lives) |
| 12 | ENEMY_UPDATE      | S→C       | Update enemy-specific state (type/size/color) |
| 13 | BULLET_UPDATE     | S→C       | Update bullet-specific state (owner/radius) |
| 14 | FULL_ENTITY_UPDATE| S→C       | Complete entity state update (combined packet) |

## Connection Handshake

```
Client                      Server
│                                │
├─ CONNECT ────────────────>     │
│                                │
│     ◀──── CONNECT_OK ─────────│
│        (includes Player ID)    │
```

## Packet Payloads

### CONNECT

- **Payload**: *None*
- Sent by client to initiate a connection.
- Server MAY silently drop or reply with `CONNECT_OK`.

### CONNECT_OK

- **Payload (4 bytes)**:
  - `Player ID` (uint32)


### INPUT

- **Payload (5 bytes)**:
  - `Player ID` (uint32)
  - `Action` (uint8):
    - `0` = NONE (stop movement)
    - `1` = UP
    - `2` = DOWN
    - `3` = LEFT
    - `4` = RIGHT
    - `5` = SHOOT
    - `6` = UP_LEFT (diagonal)
    - `7` = UP_RIGHT (diagonal)
    - `8` = DOWN_LEFT (diagonal)
    - `9` = DOWN_RIGHT (diagonal)

> /!\ Sent **on input state change** for optimal bandwidth usage.

### DISCONNECT

- **Payload**: *None*

### DISCONNECT_OK

- **Payload**: *None*

### PING

- **Payload (8 bytes)**:
  - `Timestamp` (uint64) – milliseconds since epoch

### PONG

- **Payload (8 bytes)**:
  - `Timestamp` (uint64) – same as received in PING

### ENTITY_CREATE

- **Payload (13 bytes)**:
  - `Entity ID` (uint32)
  - `Entity Type` (uint8): 0=Player, 1=Bullet, 2=Enemy
  - `X Position` (float32)
  - `Y Position` (float32)

### ENTITY_UPDATE

- **Payload (21 bytes)**:
  - `Entity ID` (uint32)
  - `X Position` (float32)
  - `Y Position` (float32)
  - `X Velocity` (float32)
  - `Y Velocity` (float32)
  - `Health` (uint8)

### ENTITY_DESTROY

- **Payload (4 bytes)**:
  - `Entity ID` (uint32)

### TRANSFORM_UPDATE

- **Payload (16 bytes)**:
  - `Entity ID` (uint32)
  - `X Position` (float32)
  - `Y Position` (float32)
  - `Rotation` (float32) - in degrees

### PLAYER_UPDATE

- **Payload (19 bytes)**:
  - `Entity ID` (uint32)
  - `Player ID` (uint32)
  - `Health` (int32)
  - `Score` (int32)
  - `Lives` (int32)
  - `Is Dead` (uint8) - boolean flag
  - `Is Invincible` (uint8) - boolean flag for respawn invincibility

### ENEMY_UPDATE

- **Payload (19 bytes)**:
  - `Entity ID` (uint32)
  - `Enemy Type` (uint8)
  - `Width` (float32) - collision and render size
  - `Height` (float32) - collision and render size
  - `Color R` (uint8) - red component (0-255)
  - `Color G` (uint8) - green component (0-255)
  - `Color B` (uint8) - blue component (0-255)

### BULLET_UPDATE

- **Payload (12 bytes)**:
  - `Entity ID` (uint32)
  - `Owner ID` (uint32) - entity ID of the shooter
  - `Radius` (float32) - collision radius

### FULL_ENTITY_UPDATE

Combined packet for complete entity synchronization. Reduces packet count by combining transform and type-specific data.

- **Payload (variable size)**:
  - `Entity ID` (uint32)
  - `Entity Type` (uint8): 0=Player, 1=Enemy, 2=Bullet
  - `X Position` (float32)
  - `Y Position` (float32)
  - `Rotation` (float32)
  - **Type-Specific Data** (varies by entity type):

**For Player (18 bytes additional)**:
- `Player ID` (uint32)
- `Health` (int32)
- `Score` (int32)
- `Lives` (int32)
- `Is Dead` (uint8)
- `Is Invincible` (uint8)

**For Enemy (15 bytes additional)**:
- `Enemy Type` (uint8)
- `Width` (float32)
- `Height` (float32)
- `Color R` (uint8)
- `Color G` (uint8)
- `Color B` (uint8)

**For Bullet (8 bytes additional)**:
- `Owner ID` (uint32)
- `Radius` (float32)

## Error Handling

### Malformed Packets

The server MUST:
- Check payload length doesn't exceed maximum
- Reject packets with invalid packet types
- Never crash on malformed data

### Buffer Overflow Prevention

- Maximum packet size: 1024 bytes
- Payload length field is 1 byte (max 1025)
- All string fields are length-prefixed
- Bounds checking on all array operations

### Network Issues

**Packet Loss**: UDP doesn't guarantee delivery
- Critical events (connect, disconnect) may need retry logic
- Non-critical updates (position) can be dropped
- Use sequence numbers to detect lost packets

**Out-of-Order Packets**:
- Check sequence numbers
- Discard outdated state updates
- Keep a small window of recent sequences

**Duplicate Packets**:
- Track recently processed sequence numbers
- Ignore duplicates within the window

## Bandwidth Optimization

### Update Rate

- Server sends STATE_UPDATE at 60 Hz
- Clients send INPUT only when it's necessary
- Critical events sent immediately

## Security Considerations

### DoS Prevention

- Rate limiting on connections per IP
- Maximum packet rate enforcement
- Connection timeout after inactivity

### Data Validation

- Bounds check all input values
- Validate enum/flag values
- Reject impossibly large entity counts

### Cheating Prevention

- Server authoritative (clients can't fake state)
- Input validation (speed limits, cooldowns)
- Sanity checks on all client data

## Testing the Protocol

Use network tools to inspect packets of R-Type:

```bash
# Capture packets on UDP port 57208
tcpdump -i any udp port 57208 -X

# Test with netcat
nc -u localhost 57208
```

## Future Extensions

Potential protocol improvements:

- **Reliable Channel**: TCP connection for critical events
- **Encryption**: TLS/DTLS for secure communication
- **Compression**: zlib or LZ4 for state updates
- **Prediction**: Client-side prediction with server reconciliation
- **Interest Management**: Send updates only for nearby entities

## Protocol Versioning

The protocol version in CONNECT allows for backward compatibility:

- Major version change: Breaking changes
- Minor version change: Backward compatible additions

Current version: **1.0**

## Implementation Notes

See the network code in:
- `server/src/systems/networkSystem/` - Server-side implementation
- `client/src/systems/networkSystem/` - Client-side implementation

## Implementation Notes

### Example: Packet Parser (Python)

```python
import struct

def parse_packet(data):
    if len(data) < 4:
        raise ValueError("Packet too short")

    pkt_type, payload_size = struct.unpack("!HH", data[:4])
    payload = data[4:4+payload_size]
    return pkt_type, payload
