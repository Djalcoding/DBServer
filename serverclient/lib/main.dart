import 'dart:math';

import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:serverclient/appinfo.dart';
import 'package:serverclient/pages.dart';
import 'package:serverclient/statusbar.dart';

final _themeNotifier = ValueNotifier<ThemeMode>(ThemeMode.light);

void main() {
  runApp(const App());
}

class App extends StatelessWidget {
  const App({super.key});

  @override
  Widget build(BuildContext context) {
    return ValueListenableBuilder(
      valueListenable: _themeNotifier,
      builder: (context, theme, child) => MaterialApp(
        title: 'Server Client',
        home: const HomePage(),
        theme: lightTheme,
        darkTheme: darkTheme,
        themeMode: theme,
      ),
    );
  }
}

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() {
    return HomePageState();
  }
}

typedef ScaffoldKey = GlobalKey<ScaffoldState>;

class HomePageState extends State<HomePage> {
  ServerPage _page = ServerPage.connect;
  ConnectivityStatus connectivityStatus = ConnectivityStatus.disconnected;
  final ScaffoldKey _scaffoldKey = ScaffoldKey();
  bool _isEnglish = true;
  final List<PlatformFile> _scheduledFiles = [];
  List<PlatformFile> get scheduledFiles => _scheduledFiles;
  void scheduleFile(PlatformFile file) => _scheduledFiles.add(file);
  void scheduleFiles(List<PlatformFile>? files) =>
      setState(() => _scheduledFiles.addAll(files ?? []));
  void removeScheduledFile(int id) =>
      setState(() => _scheduledFiles.removeAt(id));
  void clearScheduleFiles() => setState(() => _scheduledFiles.clear());

  bool get isEnglish => _isEnglish;
  bool get isDarkTheme => _themeNotifier.value == ThemeMode.dark;
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: _scaffoldKey,
      appBar: buildStatusBar(
        title: connectivityStatus.getString("127.0.0.1").get(_isEnglish),
        color: connectivityStatus.color,
        icon: connectivityStatus.icon,
        useSurfaceText:
            isDarkTheme && connectivityStatus == ConnectivityStatus.connecting,
        shadowColor: Theme.of(context).colorScheme.shadow,
      ),
      drawer: _buildDrawer(),
      body: _page.build(this),
    );
  }

  Drawer _buildDrawer({double widthPercent = 0.5}) {
    return Drawer(
      width: MediaQuery.of(context).size.width * widthPercent,
      child: Padding(
        padding: EdgeInsetsGeometry.only(top: 10),
        child: Column(
          children: [
            Text(
              "Actions",
              style: TextStyle(
                fontSize: 30,
                fontWeight: FontWeight.w600,
                color: Theme.of(context).colorScheme.onSurfaceVariant,
              ),
            ),
            Expanded(
              child: Column(
                spacing: 0,
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  _buildActionBox(
                    title: ServerPage.connect.title.get(_isEnglish),
                    icon: Icons.wifi,
                    target: ServerPage.connect,
                  ),
                  _buildActionBox(
                    title: ServerPage.sendFiles.title.get(_isEnglish),
                    icon: Icons.arrow_upward,
                    target: ServerPage.sendFiles,
                  ),
                  _buildActionBox(
                    title: ServerPage.receiveFiles.title.get(_isEnglish),
                    icon: Icons.arrow_downward,
                    target: ServerPage.receiveFiles,
                  ),
                  _buildActionBox(
                    title: ServerPage.serverInfo.title.get(_isEnglish),
                    icon: Icons.settings_suggest_sharp,
                    target: ServerPage.serverInfo,
                  ),
                  _buildActionBox(
                    title: ServerPage.settings.title.get(_isEnglish),
                    icon: Icons.settings,
                    target: ServerPage.settings,
                    bottom: true,
                  ),
                  Expanded(flex: 4, child: SizedBox()),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildActionBox({
    required String title,
    required IconData icon,
    required ServerPage target,
    double size = 0.8,
    bool bottom = false,
  }) {
    double widthCompoment = MediaQuery.of(context).size.width / 800 * size;
    Color color = Theme.of(context).colorScheme.onSurfaceVariant;
    return Expanded(
      child: Container(
        decoration: BoxDecoration(
          border: BoxBorder.fromLTRB(
            top: BorderSide(color: color.withAlpha(150), width: 1),
            bottom: bottom
                ? BorderSide(color: color.withAlpha(150), width: 1)
                : BorderSide.none,
          ),
        ),
        child: TextButton(
          onPressed: () => {
            setState(() {
              _page = target;
              _scaffoldKey.currentState!.closeDrawer();
            }),
          },
          style: TextButton.styleFrom(
            foregroundColor: color,
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadiusGeometry.circular(0),
            ),
            splashFactory: NoSplash.splashFactory,
            surfaceTintColor: Colors.black,
          ),
          child: Row(
            children: [
              Text(
                title,
                style: TextStyle(fontSize: min(35, 35 * widthCompoment)),
              ),
              Expanded(child: Container()),
              Icon(icon, size: min(35, 38 * widthCompoment)),
            ],
          ),
        ),
      ),
    );
  }

  void setLanguage(bool status) {
    setState(() {
      _isEnglish = status;
    });
  }

  void setDarkTheme(bool darkTheme) {
    setState(() {
      _themeNotifier.value = darkTheme ? ThemeMode.dark : ThemeMode.light;
    });
  }
}
