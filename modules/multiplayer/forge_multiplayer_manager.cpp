/**************************************************************************/
/*  forge_multiplayer_manager.cpp                                         */
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

#include "forge_multiplayer_manager.h"

#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "modules/enet/enet_multiplayer_peer.h"
#include "scene/main/scene_tree.h"

void ForgeMultiplayerManager::_ready() {
	multiplayer_api = MultiplayerAPI::create_default_interface();
	get_tree()->set_multiplayer(multiplayer_api);

	multiplayer_api->connect("peer_connected", callable_mp(this, &ForgeMultiplayerManager::_on_peer_connected));
	multiplayer_api->connect("peer_disconnected", callable_mp(this, &ForgeMultiplayerManager::_on_peer_disconnected));
	multiplayer_api->connect("connection_failed", callable_mp(this, &ForgeMultiplayerManager::_on_connection_failed));
	multiplayer_api->connect("connected_to_server", callable_mp(this, &ForgeMultiplayerManager::_on_connected_to_server));

	if (auto_start) {
		if (connection_mode == CONNECTION_MODE_DEDICATED_SERVER) {
			start_server(server_port, max_clients);
		} else if (connection_mode == CONNECTION_MODE_HOST) {
			start_host(server_port, max_clients);
		} else if (connection_mode == CONNECTION_MODE_CLIENT) {
			join_server(server_ip, server_port);
		}
	}
}

void ForgeMultiplayerManager::_process(double p_delta) {
	if (multiplayer_api.is_valid()) {
		multiplayer_api->poll();
	}
}

ForgeMultiplayerManager::ForgeMultiplayerManager() {
}

Error ForgeMultiplayerManager::start_server(int p_port, int p_max_clients) {
	Ref<ENetMultiplayerPeer> peer;
	peer.instantiate();
	
	Error err = peer->create_server(p_port, p_max_clients);
	if (err != OK) {
		return err;
	}

	multiplayer_peer = peer;
	multiplayer_api->set_multiplayer_peer(multiplayer_peer);
	connection_mode = CONNECTION_MODE_DEDICATED_SERVER;
	server_port = p_port;
	max_clients = p_max_clients;

	print_line(vformat("Forge Engine: Server started on port %d with max %d clients", p_port, p_max_clients));
	return OK;
}

Error ForgeMultiplayerManager::start_host(int p_port, int p_max_clients) {
	Ref<ENetMultiplayerPeer> peer;
	peer.instantiate();
	
	Error err = peer->create_server(p_port, p_max_clients);
	if (err != OK) {
		return err;
	}

	multiplayer_peer = peer;
	multiplayer_api->set_multiplayer_peer(multiplayer_peer);
	connection_mode = CONNECTION_MODE_HOST;
	server_port = p_port;
	max_clients = p_max_clients;

	print_line(vformat("Forge Engine: Host started on port %d with max %d clients", p_port, p_max_clients));
	return OK;
}

Error ForgeMultiplayerManager::join_server(const String &p_ip, int p_port) {
	Ref<ENetMultiplayerPeer> peer;
	peer.instantiate();
	
	Error err = peer->create_client(p_ip, p_port);
	if (err != OK) {
		return err;
	}

	multiplayer_peer = peer;
	multiplayer_api->set_multiplayer_peer(multiplayer_peer);
	connection_mode = CONNECTION_MODE_CLIENT;
	server_ip = p_ip;
	server_port = p_port;

	print_line(vformat("Forge Engine: Connecting to server %s:%d", p_ip, p_port));
	return OK;
}

void ForgeMultiplayerManager::disconnect_multiplayer() {
	if (multiplayer_peer.is_valid()) {
		multiplayer_peer->close();
		multiplayer_peer.unref();
		multiplayer_api->set_multiplayer_peer(nullptr);
		connection_mode = CONNECTION_MODE_CLIENT;
		print_line("Forge Engine: Disconnected from multiplayer");
	}
}

bool ForgeMultiplayerManager::is_server() const {
	return connection_mode == CONNECTION_MODE_DEDICATED_SERVER;
}

bool ForgeMultiplayerManager::is_host() const {
	return connection_mode == CONNECTION_MODE_HOST;
}

bool ForgeMultiplayerManager::is_client() const {
	return connection_mode == CONNECTION_MODE_CLIENT && is_connected();
}

bool ForgeMultiplayerManager::is_connected() const {
	return multiplayer_api.is_valid() && multiplayer_api->has_multiplayer_peer();
}

int ForgeMultiplayerManager::get_unique_id() const {
	if (multiplayer_api.is_valid()) {
		return multiplayer_api->get_unique_id();
	}
	return 0;
}

PackedInt32Array ForgeMultiplayerManager::get_connected_peers() const {
	if (multiplayer_api.is_valid()) {
		return multiplayer_api->get_peer_ids();
	}
	return PackedInt32Array();
}

void ForgeMultiplayerManager::set_server_port(int p_port) {
	server_port = p_port;
}

int ForgeMultiplayerManager::get_server_port() const {
	return server_port;
}

void ForgeMultiplayerManager::set_server_ip(const String &p_ip) {
	server_ip = p_ip;
}

String ForgeMultiplayerManager::get_server_ip() const {
	return server_ip;
}

void ForgeMultiplayerManager::set_max_clients(int p_max) {
	max_clients = p_max;
}

int ForgeMultiplayerManager::get_max_clients() const {
	return max_clients;
}

void ForgeMultiplayerManager::set_auto_start(bool p_enable) {
	auto_start = p_enable;
}

bool ForgeMultiplayerManager::get_auto_start() const {
	return auto_start;
}

void ForgeMultiplayerManager::_on_peer_connected(int p_peer_id) {
	print_line(vformat("Forge Engine: Peer %d connected", p_peer_id));
	emit_signal("peer_connected", p_peer_id);
}

void ForgeMultiplayerManager::_on_peer_disconnected(int p_peer_id) {
	print_line(vformat("Forge Engine: Peer %d disconnected", p_peer_id));
	emit_signal("peer_disconnected", p_peer_id);
}

void ForgeMultiplayerManager::_on_connection_failed() {
	print_line("Forge Engine: Connection failed");
	emit_signal("connection_failed");
}

void ForgeMultiplayerManager::_on_connected_to_server() {
	print_line(vformat("Forge Engine: Connected to server with ID %d", get_unique_id()));
	emit_signal("connected_to_server");
}

void ForgeMultiplayerManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_server", "port", "max_clients"), &ForgeMultiplayerManager::start_server, DEFVAL(7000), DEFVAL(32));
	ClassDB::bind_method(D_METHOD("start_host", "port", "max_clients"), &ForgeMultiplayerManager::start_host, DEFVAL(7000), DEFVAL(32));
	ClassDB::bind_method(D_METHOD("join_server", "ip", "port"), &ForgeMultiplayerManager::join_server, DEFVAL("127.0.0.1"), DEFVAL(7000));
	ClassDB::bind_method(D_METHOD("disconnect_multiplayer"), &ForgeMultiplayerManager::disconnect_multiplayer);

	ClassDB::bind_method(D_METHOD("is_server"), &ForgeMultiplayerManager::is_server);
	ClassDB::bind_method(D_METHOD("is_host"), &ForgeMultiplayerManager::is_host);
	ClassDB::bind_method(D_METHOD("is_client"), &ForgeMultiplayerManager::is_client);
	ClassDB::bind_method(D_METHOD("is_connected"), &ForgeMultiplayerManager::is_connected);
	ClassDB::bind_method(D_METHOD("get_unique_id"), &ForgeMultiplayerManager::get_unique_id);
	ClassDB::bind_method(D_METHOD("get_connected_peers"), &ForgeMultiplayerManager::get_connected_peers);

	ClassDB::bind_method(D_METHOD("set_server_port", "port"), &ForgeMultiplayerManager::set_server_port);
	ClassDB::bind_method(D_METHOD("get_server_port"), &ForgeMultiplayerManager::get_server_port);
	ClassDB::bind_method(D_METHOD("set_server_ip", "ip"), &ForgeMultiplayerManager::set_server_ip);
	ClassDB::bind_method(D_METHOD("get_server_ip"), &ForgeMultiplayerManager::get_server_ip);
	ClassDB::bind_method(D_METHOD("set_max_clients", "max_clients"), &ForgeMultiplayerManager::set_max_clients);
	ClassDB::bind_method(D_METHOD("get_max_clients"), &ForgeMultiplayerManager::get_max_clients);
	ClassDB::bind_method(D_METHOD("set_auto_start", "enable"), &ForgeMultiplayerManager::set_auto_start);
	ClassDB::bind_method(D_METHOD("get_auto_start"), &ForgeMultiplayerManager::get_auto_start);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "server_port"), "set_server_port", "get_server_port");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_ip"), "set_server_ip", "get_server_ip");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_clients"), "set_max_clients", "get_max_clients");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_start"), "set_auto_start", "get_auto_start");

	ADD_SIGNAL(MethodInfo("peer_connected", PropertyInfo(Variant::INT, "peer_id")));
	ADD_SIGNAL(MethodInfo("peer_disconnected", PropertyInfo(Variant::INT, "peer_id")));
	ADD_SIGNAL(MethodInfo("connection_failed"));
	ADD_SIGNAL(MethodInfo("connected_to_server"));

	BIND_ENUM_CONSTANT(CONNECTION_MODE_DEDICATED_SERVER);
	BIND_ENUM_CONSTANT(CONNECTION_MODE_HOST);
	BIND_ENUM_CONSTANT(CONNECTION_MODE_CLIENT);
}
