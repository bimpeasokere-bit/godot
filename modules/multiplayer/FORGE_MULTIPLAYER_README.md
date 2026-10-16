# Forge Engine One-Click Multiplayer

Forge Engine comes with a pre-configured multiplayer framework that allows developers to prototype online games instantly.

## Quick Start

### Option 1: Using the ForgeMultiplayerManager Node

1. In your scene, add a `ForgeMultiplayerManager` node
2. Configure it in the Inspector:
   - **Server Port**: Default 7000
   - **Server IP**: Default "127.0.0.1"
   - **Max Clients**: Default 32
   - **Auto Start**: Enable to auto-connect on scene load

3. Use these methods in your script:

```gdscript
# Start as dedicated server
multiplayer_manager.start_server(7000, 32)

# Start as host (server + client)
multiplayer_manager.start_host(7000, 32)

# Join as client
multiplayer_manager.join_server("127.0.0.1", 7000)

# Disconnect
multiplayer_manager.disconnect_multiplayer()
```

### Option 2: Using the Template

Copy the `forge_multiplayer_template.gd` file to your project and adapt it to your needs.

## API Reference

### ForgeMultiplayerManager

#### Methods

**`start_server(port: int = 7000, max_clients: int = 32) -> Error`**
- Starts a dedicated server
- Returns OK on success, error code on failure

**`start_host(port: int = 7000, max_clients: int = 32) -> Error`**
- Starts a host (server + client in one)
- Returns OK on success, error code on failure

**`join_server(ip: String = "127.0.0.1", port: int = 7000) -> Error`**
- Joins an existing server
- Returns OK on success, error code on failure

**`disconnect_multiplayer()`**
- Disconnects from the current multiplayer session

**`is_server() -> bool`**
- Returns true if running as dedicated server

**`is_host() -> bool`**
- Returns true if running as host

**`is_client() -> bool`**
- Returns true if connected as client

**`is_connected() -> bool`**
- Returns true if connected to multiplayer

**`get_unique_id() -> int`**
- Returns the unique peer ID for this instance

**`get_connected_peers() -> PackedInt32Array`**
- Returns array of connected peer IDs

#### Properties

**`server_port: int`**
- The port to use for server/host (default: 7000)

**`server_ip: String`**
- The IP address to connect to (default: "127.0.0.1")

**`max_clients: int`**
- Maximum number of clients (default: 32)

**`auto_start: bool`**
- Automatically start connection on scene load (default: false)

#### Signals

**`peer_connected(peer_id: int)`**
- Emitted when a new peer connects

**`peer_disconnected(peer_id: int)`**
- Emitted when a peer disconnects

**`connection_failed()`**
- Emitted when connection attempt fails

**`connected_to_server()`**
- Emitted when successfully connected to server

## Network Optimizations

Forge Engine comes pre-configured with optimized network settings:

- **Connection Timeout**: 15 seconds (reduced from 30 for faster failover)
- **Max Packet Size**: 32KB (optimized for modern networks)
- **Max Channels**: 32 (supports multiple data streams)
- **Buffer Size**: 1MB (reduced from 64KB for better throughput)
- **Replication Rate**: 20Hz (20 updates per second)
- **Physics Replication Rate**: 50Hz (50 updates per second)
- **Compression**: Enabled by default
- **Max Sync Count**: 64 (number of synchronized nodes)

## Best Practices

### 1. Player Spawning

```gdscript
@rpc("any_peer", "call_remote", "reliable")
func spawn_player(peer_id: int):
    # Only server should spawn players
    if not multiplayer_manager.is_server():
        return
    
    var player = preload("res://player.tscn").instantiate()
    player.set_multiplayer_authority(peer_id)
    player.name = str(peer_id)
    $Players.add_child(player)
```

### 2. Synchronized Movement

```gdscript
# On player node
@export var speed: float = 5.0

@rpc("any_peer", "call_remote", "unreliable")
func set_position(pos: Vector2):
    if multiplayer_manager.get_remote_sender_id() == get_multiplayer_authority():
        global_position = pos

func _process(delta):
    if is_multiplayer_authority():
        # Local input
        if Input.is_action_pressed("ui_up"):
            position.y -= speed * delta
        
        # Send position to others
        set_position.rpc(position)
```

### 3. Game State Sync

```gdscript
# Using MultiplayerSynchronizer (built-in Godot node)
# Add it as a child of the node you want to sync
# Configure:
# - Replication Mode: "If Owner" or "Always"
# - Select properties to sync in the editor
```

## Complete Example

```gdscript
extends Node

@onready var multiplayer_manager = $ForgeMultiplayerManager
@onready var players_node = $Players

func _ready():
    multiplayer_manager.peer_connected.connect(_on_peer_connected)
    multiplayer_manager.peer_disconnected.connect(_on_peer_disconnected)
    multiplayer_manager.connected_to_server.connect(_on_connected_to_server)
    
    # Auto-start as host for quick testing
    multiplayer_manager.start_host(7000, 32)

func _on_peer_connected(peer_id: int):
    print("Player joined: ", peer_id)
    spawn_player.rpc_id(peer_id, peer_id)

func _on_peer_disconnected(peer_id: int):
    print("Player left: ", peer_id)
    if players_node.has_node(str(peer_id)):
        players_node.get_node(str(peer_id)).queue_free()

func _on_connected_to_server():
    print("Connected!")
    spawn_player.rpc_id(1, multiplayer_manager.get_unique_id())

@rpc("any_peer", "call_remote", "reliable")
func spawn_player(peer_id: int):
    var player = preload("res://player.tscn").instantiate()
    player.set_multiplayer_authority(peer_id)
    player.name = str(peer_id)
    players_node.add_child(player)
```

## Troubleshooting

### Connection Failed
- Check firewall settings
- Ensure port is not blocked
- Verify IP address is correct
- Check if server is actually running

### Lag/High Latency
- Reduce replication rate in Project Settings
- Use unreliable RPC for frequent updates
- Enable compression (enabled by default)
- Use client-side prediction for movement

### Players Not Syncing
- Ensure `set_multiplayer_authority()` is called
- Check replication config on MultiplayerSynchronizer
- Verify RPC annotations are correct
- Check that nodes are in the scene tree

## Advanced Features

### Lobby System

```gdscript
# Simple lobby implementation
var lobby_players = {}

@rpc("any_peer", "call_remote", "reliable")
func register_player(player_id: int, player_name: String):
    if multiplayer_manager.is_server():
        lobby_players[player_id] = player_name
        update_lobby.rpc(lobby_players)

@rpc("authority", "call_remote", "reliable")
func update_lobby(players_data: Dictionary):
    lobby_players = players_data
    # Update UI
```

### Matchmaking

```gdscript
# Basic matchmaking via master server
func find_match():
    var http = HTTPRequest.new()
    add_child(http)
    http.request_completed.connect(_on_match_found)
    http.request("https://your-matchmaking-server.com/find_match")

func _on_match_found(result, response_code, headers, body):
    var json = JSON.new()
    var match_data = json.parse_string(body.get_string_from_utf8())
    multiplayer_manager.join_server(match_data.ip, match_data.port)
```

## Performance Tips

1. **Use unreliable RPC** for frequent updates (position, rotation)
2. **Use reliable RPC** for critical events (spawning, state changes)
3. **Batch RPC calls** when possible
4. **Enable compression** for large data
5. **Limit sync rate** based on importance
6. **Use client-side prediction** for movement
7. **Implement interpolation** for smooth updates

## Security Considerations

- Always validate client input on the server
- Use authority checks in RPC functions
- Never trust client-side calculations
- Implement rate limiting for RPC calls
- Sanitize all user input
- Use encryption for sensitive data

## Support

For more information, see:
- Godot Multiplayer Documentation: https://docs.godotengine.org
- Forge Engine Community: https://forgeengine.io/community
