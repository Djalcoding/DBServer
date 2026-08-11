import 'package:flutter/material.dart';
import 'package:serverclient/main.dart';
import 'package:serverclient/pages.dart';

Widget settingsPage(HomePageState state) {
  return Column(
    spacing: 20,
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text(
        ServerPage.settings.title.get(state.isEnglish),
        style: TextStyle(fontSize: 50, fontWeight: FontWeight.bold),
      ),
      SizedBox(height: 10),
      _buildSwitchField(
        "Language",
        value: state.isEnglish,
        pin: Pin(
          color: Colors.orange,
          icon: Icons.language,
          padding: EdgeInsetsGeometry.all(5),
          size: 40,
        ),
        onChanged: state.setLanguage,
        activatedIcon: Icons.language,
        deactivatedIcon: Icons.abc,
      ),

      _buildSwitchField(
        "Dark Theme",
        value: state.isDarkTheme,
        pin: Pin(
          color: Colors.blue,
          icon: Icons.dark_mode_rounded,
          padding: EdgeInsetsGeometry.all(5),
          size: 40,
        ),
        onChanged: state.setDarkTheme,
        activatedIcon: Icons.dark_mode,
        deactivatedIcon: Icons.light_mode,
      ),
      _buildSwitchField(
        "Save connection keys",
        value: true,
        onChanged: (bool b) {},
        activatedIcon: Icons.check,
        deactivatedIcon: Icons.close_rounded,
        pin: Pin(
          color: Colors.grey,
          icon: Icons.vpn_key_rounded,
          size: 40,
          padding: EdgeInsetsDirectional.all(5),
        ),
      ),
    ],
  );
}

Widget _buildSwitchField(
  String name, {
  required bool value,
  required void Function(bool) onChanged,
  required IconData activatedIcon,
  required IconData deactivatedIcon,
  required Pin pin,
}) {
  return Builder(
    builder: (BuildContext context) {
      ThemeData theme = Theme.of(context);
      return Row(
        spacing: 10,
        children: [
          pin,
          Text(
            name,
            style: TextStyle(fontSize: 30, color: theme.colorScheme.onSurface),
          ),
          Expanded(child: SizedBox()),
          Transform.scale(
            scale: 1.15,
            child: Switch(
              trackColor: WidgetStateProperty.fromMap({
                WidgetState.selected: Colors.blue,
              }),
              value: value,
              onChanged: onChanged,

              thumbIcon: WidgetStateProperty<Icon>.fromMap(
                <WidgetStatesConstraint, Icon>{
                  WidgetState.selected: Icon(activatedIcon),
                  WidgetState.any: Icon(deactivatedIcon),
                },
              ),
            ),
          ),
        ],
      );
    },
  );
}
