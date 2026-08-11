import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';

import 'package:serverclient/statusbar.dart';

typedef ConnectivityListener = void Function(ConnectivityStatus newStatus);

class ServerPipe {
  Socket? socket;
  Stream<Uint8List>? dataStream;
  ConnectivityStatus _connectivityStatus = ConnectivityStatus.disconnected;
  String? errorMessage;
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
      onDone: () => _setConnectivity(ConnectivityStatus.disconnected),
    );
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

  void httpGET(String url) {
    write("GET $url HTTP/1.1");
  }

  void drop() async {
    await socket!.close();
  }
}
