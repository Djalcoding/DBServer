import 'dart:async';
import 'dart:io';
import 'dart:typed_data';

import 'package:serverclient/statusbar.dart';

typedef ConnectivityListener = void Function(ConnectivityStatus newStatus);

class ServerPipe {
  Socket? socket;
  Stream<Uint8List>? dataStream;
  ConnectivityStatus _connectivityStatus = ConnectivityStatus.disconnected;
  String? errorMessage;
  String? host;
  int? port;

  String _authorization = "";
  final List<ConnectivityListener> _connectivityListeners = [];

  Future<void> connect({required String host, required int port}) async {
    _setConnectivity(ConnectivityStatus.connecting);
    try {
      socket = await Socket.connect(host, port);
    } catch (e) {
      _setConnectivity(ConnectivityStatus.failure);
      errorMessage = e.toString();
      return;
    }
    _setConnectivity(ConnectivityStatus.connected);
    socket!.listen(
      null,
      onDone: () {
        _setConnectivity(ConnectivityStatus.disconnected);
        this.host = null;
        this.port = null;
      },
    );
    this.host = host;
    this.port = port;
  }

  ConnectivityStatus get connectivity {
    return _connectivityStatus;
  }

  void _setConnectivity(ConnectivityStatus state) {
    _connectivityStatus = state;
    for (var listener in _connectivityListeners) {
      listener(_connectivityStatus);
    }
  }

  void onConnectivityUpdate(
    void Function(ConnectivityStatus newStatus) callback,
  ) {
    _connectivityListeners.add(callback);
  }

  void write(String data) {
    if (socket == null) throw Exception("Socket isn't connected");
    socket!.write(data);
  }

  void httpGET(String url, {bool keepAlive = false}) {
    String requestLine = "GET $url HTTP/1.1";
    String hostHTTP = "HOST: $host:$port";
    String connection = "Connection: ${keepAlive ? "keep-alive" : "close"}";
    write("$requestLine\r\n$hostHTTP\r\n$connection\r\n");
  }

  void drop() async {
    await socket!.close();
  }

  void setAuthorization(String data) {
    _authorization = data;
  }
}
