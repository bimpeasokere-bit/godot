# Forge Engine One-Click Multiplayer Template
# This template demonstrates how to quickly set up multiplayer in Forge Engine

extends Node

# Add ForgeMultiplayerManager as a child node in your scene
@onready var multiplayer_manager = $ForgeMultiplayerManager

func _ready():
	# Connect to multiplayer signals
	multiplayer_manager.peer_connected.connect(_on_peer_connected)
	multiplayer_manager.peer_disconnected.connect(_on_peer_disconnected)
	multiplayer_manager.connection_failed.connect(_on_connection_failed)
	multiplayer_manager.connected_to_server.connect(_on_connected_to_server)

	# Option 1: Auto-start based on project settings
	# Set in editor: multiplayer_manager.auto_start = true
	# Set in editor: multiplayer_manager.server_port = 7000
	# Set in editor: multiplayer_manager.server_ip = "127.0.0.1"

	# Option 2: Manual start (comment out auto_start in editor)
	# Start as server
	# multiplayer_manager.start_server(7000, 32)
	
	# Start as host (server + client)
	# multiplayer_manager.start_host(7000, 32)
	
	# Join as client
	# multiplayer_manager.join_server("127.0.0.1", 7000)

func _process(_delta):
	# Check connection status
	if multiplayer_manager.is_connected():
		var my_id = multiplayer_manager.get_unique_id()
		var peers = multiplayer_manager.get_connected_peers()
		print("My ID: ", my_id, " | Connected peers: ", peers)

# Callbacks
func _on_peer_connected(peer_id):
	print("Player ", peer_id, " connected!")
	# Spawn player for this peer
	_spawn_player(peer_id)

func _on_peer_disconnected(peer_id):
	print("Player ", peer_id, " disconnected!")
	# Remove player
	_despawn_player(peer_id)

func _on_connection_failed():
	print("Failed to connect to server!")

func _on_connected_to_server():
	print("Successfully connected to server!")
	# Spawn local player
	_spawn_player(multiplayer_manager.get_unique_id())

# Player spawning logic
func _spawn_player(peer_id):
	# Instantiate your player scene
	var player_scene = preload("res://player.tscn")
	var player = player_scene.instantiate()
	
	# Set up multiplayer replication
	player.set_multiplayer_authority(peer_id)
	player.name = str(peer_id)
	
	# Add to scene
	$Players.add_child(player)

func _despawn_player(peer_id):
	if $Players.has_node(str(peer_id)):
		$Players.get_node(str(peer_id)).queue_free()

# RPC Example - Call this from any node with @rpc annotation
@rpc("any_peer", "call_remote", "reliable")
func send_chat_message(message: String, sender_id: int):
	print("Player ", sender_id, " says: ", message)
	# Display message in chat UI
