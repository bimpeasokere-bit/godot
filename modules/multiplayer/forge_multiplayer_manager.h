/**************************************************************************/
/*  forge_multiplayer_manager.h                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             FORGE ENGINE                               */
/*                        https://forgeengine.io                          */
/**************************************************************************/
/* Copyright (c) 2024-present Forge Engine contributors.                 */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR   */
/* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "scene/main/node.h"
#include "scene/main/multiplayer_api.h"
#include "scene/main/multiplayer_peer.h"

class ForgeMultiplayerManager : public Node {
	GDCLASS(ForgeMultiplayerManager, Node);

public:
	enum ConnectionMode {
		CONNECTION_MODE_DEDICATED_SERVER,
		CONNECTION_MODE_HOST,
		CONNECTION_MODE_CLIENT
	};

protected:
	static void _bind_methods();

private:
	Ref<MultiplayerAPI> multiplayer_api;
	Ref<MultiplayerPeer> multiplayer_peer;
	ConnectionMode connection_mode = CONNECTION_MODE_CLIENT;
	int server_port = 7000;
	String server_ip = "127.0.0.1";
	int max_clients = 32;
	bool auto_start = false;

	void _ready();
	void _process(double p_delta);

public:
	ForgeMultiplayerManager();

	// One-Click Server Setup
	Error start_server(int p_port = 7000, int p_max_clients = 32);
	Error start_host(int p_port = 7000, int p_max_clients = 32);
	Error join_server(const String &p_ip = "127.0.0.1", int p_port = 7000);
	void disconnect_multiplayer();

	// Connection State
	bool is_server() const;
	bool is_host() const;
	bool is_client() const;
	bool is_connected() const;
	int get_unique_id() const;
	PackedInt32Array get_connected_peers() const;

	// Configuration
	void set_server_port(int p_port);
	int get_server_port() const;
	void set_server_ip(const String &p_ip);
	String get_server_ip() const;
	void set_max_clients(int p_max);
	int get_max_clients() const;
	void set_auto_start(bool p_enable);
	bool get_auto_start() const;

	// Signals
	void _on_peer_connected(int p_peer_id);
	void _on_peer_disconnected(int p_peer_id);
	void _on_connection_failed();
	void _on_connected_to_server();
};

VARIANT_ENUM_CAST(ForgeMultiplayerManager::ConnectionMode);
