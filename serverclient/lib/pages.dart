import 'dart:math';

import 'package:flutter/material.dart';
import 'package:serverclient/main.dart';
import 'package:serverclient/pages/send.dart';
import 'package:serverclient/pages/connect.dart';
import 'package:serverclient/pages/serverinfo.dart';
import 'package:serverclient/pages/settings.dart';
import 'package:serverclient/translation.dart';

class Pin extends StatelessWidget {
  final Color color;
  final IconData icon;
  final EdgeInsetsGeometry? padding;
  final double size;
  const Pin({
    super.key,
    required this.color,
    required this.icon,
    this.padding,
    this.size = 50,
  });
  @override
  Widget build(BuildContext context) {
    return Container(
      padding: padding,
      decoration: BoxDecoration(
        color: color.withAlpha(100),
        borderRadius: BorderRadius.all(Radius.circular(50)),
      ),
      child: Icon(icon, size: size, color: color),
    );
  }
}

enum ServerPage {
  connect(BilingualString("Connect", "Se Connecter"), widget: connectPage),
  sendFiles(
    BilingualString("Send files", "Envoyer des fichiers"),
    widget: sendFilesPage,
    needsConnection: true,
  ),
  receiveFiles(
    BilingualString("Receive files", "Recevoir des fichiers"),
    widget: _receiveFilesPage,
    needsConnection: true,
  ),
  serverInfo(
    BilingualString("Server information", "Informations du serveur"),
    widget: serverInfoPage,
    needsConnection: true,
  ),
  settings(BilingualString("Settings", "Paramètres"), widget: settingsPage);

  final BilingualString title;
  final Widget Function(HomePageState) _widget;
  final bool needsConnection;
  const ServerPage(
    this.title, {
    required this._widget,
    this.needsConnection = false,
  });
  Widget build(HomePageState state) {
    return Padding(
      padding: EdgeInsetsGeometry.all(30),
      child: !state.connectedToServer && needsConnection
          ? buildNotConnected()
          : _widget(state),
    );
  }

  static Widget buildNotConnected() {
    return Builder(
      builder: (BuildContext context) => Center(
        // TODO : replace all Themes and MediaQuery with variables when im not a lazy bum
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              "Server isn't connected",
              style: TextStyle(
                fontWeight: FontWeight.w500,
                fontSize: 30,
                color: Theme.of(context).colorScheme.onSurface.withAlpha(200),
              ),
            ),
              Icon(
                Icons.warning_rounded,
                size:
                    min(
                      MediaQuery.of(context).size.width,
                      MediaQuery.of(context).size.height,
                    ) /
                    1.75,
                color: Colors.deepOrange,
              ),
          ],
        ),
      ),
    );
  }

  static Widget _receiveFilesPage(HomePageState state) {
    return Center(child: Text("Receive Files"));
  }

  static Widget _serverInformationPage(HomePageState state) {
    return Center(child: Text("Server Information"));
  }
}
