import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:serverclient/filemanagement.dart';
import 'package:serverclient/main.dart';
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
  sendFiles(
    BilingualString("Send files", "Envoyer des fichiers"),
    widget: _sendFilesPage,
  ),
  receiveFiles(
    BilingualString("Receive files", "Recevoir des fichiers"),
    widget: _receiveFilesPage,
  ),
  connect(BilingualString("Connect", "Se Connecter"), widget: _connectPage),
  serverInfo(
    BilingualString("Server information", "Informations du serveur"),
    widget: _serverInformationPage,
  ),
  settings(BilingualString("Settings", "Paramètres"), widget: _settingsPage);

  final BilingualString title;
  final Widget Function(HomePageState) _widget;
  const ServerPage(this.title, {required this._widget});
  Widget build(HomePageState state) {
    return Padding(padding: EdgeInsetsGeometry.all(30), child: _widget(state));
  }

  static Widget _connectPage(HomePageState state) {
    return Column(
      children: [
        Text(
          "Connect",
          style: TextStyle(fontSize: 50, fontWeight: FontWeight.bold),
        ),
           TextField(
            decoration: InputDecoration(
              labelText: "Server Ip",
              filled: true,
              border: OutlineInputBorder(),
            ),
          ),
      ],
    );
  }

  static Widget _receiveFilesPage(HomePageState state) {
    return Center(child: Text("Receive Files"));
  }

  static Widget _sendFilePagelowerBlock(
    HomePageState state, {
    int maxLength = 10,
  }) {
    List<Widget> fileList = FileCard.fromList(
      state.scheduledFiles,
      deleteFunction: state.removeScheduledFile,
    );
    if (fileList.length > maxLength) {
      int originalLength = fileList.length;
      fileList = fileList.sublist(0, maxLength);
      fileList.add(
        FileCard.buildBox(
          radius: 10,
          width: 50,
          height: 40,
          child: Center(child: Text("+${originalLength - maxLength}")),
        ),
      );
    }
    return Builder(
      builder: (BuildContext context) {
        ThemeData theme = Theme.of(context);
        return Column(
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          spacing: 20,
          children: [
            if (fileList.isNotEmpty)
              Text(
                "Scheduled Files",
                style: TextStyle(fontSize: 35, fontWeight: FontWeight.w500),
              ),
            Wrap(spacing: 9, runSpacing: 7, children: fileList),
            SizedBox(height: 30),
            if (fileList.isNotEmpty)
              ElevatedButton(
                onPressed: state.clearScheduleFiles,

                style: ButtonStyle(
                  elevation: WidgetStatePropertyAll(3),
                  splashFactory: NoSplash.splashFactory,
                  backgroundColor: WidgetStatePropertyAll(
                    theme.colorScheme.surfaceContainerHighest,
                  ),
                  shape: WidgetStatePropertyAll(
                    RoundedRectangleBorder(
                      borderRadius: BorderRadiusGeometry.all(
                        Radius.circular(7.5),
                      ),
                    ),
                  ),
                ),
                child: Text(
                  "Clear",
                  style: TextStyle(
                    fontSize: 30,
                    color: theme.colorScheme.onSurface,
                  ),
                ),
              ),
            Expanded(child: SizedBox()),
            Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Expanded(
                  child: ElevatedButton(
                    onPressed: fileList.isNotEmpty ? () {} : null,
                    style: ButtonStyle(
                      elevation: WidgetStatePropertyAll(20),
                      backgroundColor: WidgetStateColor.fromMap({
                        WidgetState.disabled: theme.disabledColor,
                        WidgetState.any: Colors.green,
                      }),
                      shape: WidgetStatePropertyAll(
                        RoundedRectangleBorder(
                          borderRadius: BorderRadiusGeometry.all(
                            Radius.circular(7.5),
                          ),
                        ),
                      ),
                    ),
                    child: Text(
                      "Send",
                      style: TextStyle(
                        fontSize: 60,
                        color:
                            theme.colorScheme.secondaryFixed
                            , // TODO : darker text
                      ),
                    ),
                  ),
                ),
              ],
            ),
            SizedBox(height: 30),
          ],
        );
      },
    );
  }

  static Widget _sendFilesPage(HomePageState state) {
    return Builder(
      builder: (BuildContext context) {
        ThemeData theme = Theme.of(context);
        return Column(
          spacing: 20,
          mainAxisAlignment: MainAxisAlignment.start,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Expanded(
                  child: buildSendFileButton(theme, () async {
                    state.scheduleFiles(await getReceivedFiles());
                  }),
                ),
              ],
            ),
            SizedBox(height: 30),
            Expanded(child: _sendFilePagelowerBlock(state)),
          ],
        );
      },
    );
  }

  static Widget _settingsPage(HomePageState state) {
    return Column(
      spacing: 20,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          settings.title.get(state.isEnglish),
          style: TextStyle(fontSize: 50, fontWeight: FontWeight.bold),
        ),
        SizedBox(height: 10),
        _buildSwitchField(
          "Langue",
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
      ],
    );
  }

  static Widget _buildSwitchField(
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
              style: TextStyle(
                fontSize: 30,
                color: theme.colorScheme.onSurface,
              ),
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

  static const WidgetStateProperty<Icon>
  _languageIcon = //TODO : change this to display image
  WidgetStateProperty<Icon>.fromMap(<WidgetStatesConstraint, Icon>{
    WidgetState.selected: Icon(Icons.text_format),
    WidgetState.any: Icon(Icons.text_fields),
  });

  static Widget _serverInformationPage(HomePageState state) {
    return Center(child: Text("Server Information"));
  }
}
