import 'package:flutter/material.dart';
import 'package:serverclient/translation.dart';

enum ConnectivityStatus {
  connected(Colors.green),
  connecting(Colors.yellow, icon: Icons.sync_alt_rounded),
  disconnected(Colors.red, icon: Icons.wifi_off_outlined),
  failure(Color.fromARGB(255, 200, 20, 20), icon: Icons.heart_broken);

  final Color color;
  final IconData? icon;
  const ConnectivityStatus(this.color, {this.icon});

  BilingualString getString(String? ip) {
    switch (this) {
      case connected:
        return BilingualString("Connected to $ip", "Connecter à $ip");
      case connecting:
        return BilingualString("Connecting to $ip ...", "Connexion à $ip ...");
      case disconnected:
        return BilingualString("Not connected to any server", "Pas connecté");
      case failure:
        return BilingualString(
          "Failed to connect to $ip",
          "Échec de connexion à $ip",
        );
    }
  }
}

AppBar buildStatusBar({
  required String title,
  required Color color,
  required bool useSurfaceText,
  IconData? icon,
  Color? shadowColor,
}) {
  return AppBar(
    backgroundColor: color,
    elevation: 10,
    shadowColor: shadowColor,
    leading: Builder(
      builder: (BuildContext context) {
        ThemeData theme = Theme.of(context);
        return IconButton(
          iconSize: 27,
          onPressed: () => Scaffold.of(context).openDrawer(),
          style: ButtonStyle(
            shape: _drawerButtonStyle,
            elevation: WidgetStatePropertyAll(20),
            splashFactory: NoSplash.splashFactory,
            backgroundColor: WidgetStatePropertyAll(theme.colorScheme.surface),
            shadowColor: WidgetStatePropertyAll(theme.colorScheme.shadow),
          ),
          icon: Icon(Icons.menu_open, color: theme.colorScheme.onSurface),
        );
      },
    ),
    title: Builder(
      builder: (BuildContext context) {
        ThemeData theme = Theme.of(context);
        Color color = useSurfaceText
            ? theme.colorScheme.surface
            : theme.colorScheme.onSurface;
        return Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            FittedBox(
              fit: BoxFit.cover,
              child: Text(title, style: TextStyle(color: color)),
            ),
            SizedBox(width: icon == null ? 0 : 10),
            Icon(icon, size: 30, color: color),
          ],
        );
      },
    ),
  );
}

WidgetStateProperty<OutlinedBorder> _drawerButtonStyle =
    WidgetStateProperty.fromMap({
      WidgetState.any: RoundedRectangleBorder(
        borderRadius: BorderRadiusGeometry.horizontal(
          right: Radius.circular(20),
        ),
      ),
    });
