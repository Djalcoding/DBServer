import 'dart:math';

import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';

Future<List<PlatformFile>?> getReceivedFiles() async {
  FilePickerResult? result = await FilePicker.platform.pickFiles(
    allowMultiple: true,
  );
  return result?.files;
}

class FileCard extends StatelessWidget {
  final PlatformFile file;
  final void Function() deleteFunction;
  late final String name;
  FileCard({super.key, required this.file, required this.deleteFunction}) {
    int divider = 1;
    String unit = "B";
    switch (file.size) {
      case >= 1_000_000_000:
        unit = "GB";
        divider = 1_000_000_000;
        break;
      case >= 1_000_000:
        divider = 1_000_000;
        unit = "MB";
        break;
      case >= 1_000:
        divider = 1_000;
        unit = "KB";
        break;
    }
    name =
        "${file.name} (${(file.size / divider).toStringAsFixed(unit == "B" ? 0 : 1)}$unit)";
  }
  static List<Widget> fromList(
    List<PlatformFile> files, {
    required void Function(int) deleteFunction,
  }) {
    List<Widget> result = [];
    for (int i = 0; i < files.length; i++) {
      result.add(
        FileCard(
          file: files[i],
          deleteFunction: () {
            deleteFunction(i);
          },
        ),
      );
    }
    return result;
  }

  static Widget buildBox({
    double? width,
    double? height,
    double radius = 5,
    required Widget child,
  }) {
    return Builder(
      builder: (BuildContext context) {
        ThemeData theme = Theme.of(context);
        return Container(
          padding: EdgeInsets.symmetric(horizontal: 5),
          width: width,
          height: height,
          decoration: BoxDecoration(
            color: theme.colorScheme.surfaceContainerHigh,
            borderRadius: BorderRadiusGeometry.all(Radius.circular(radius)),
            boxShadow: [
              BoxShadow(
                color: theme.colorScheme.shadow.withAlpha(100),
                blurRadius: 2,
                offset: Offset(0, 3),
              ),
            ],
          ),
          child: child,
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return buildBox(
      width: min(435, max(225, name.length * 8.5)),
      child: Row(
        children: [
          Expanded(
            child: Text(name, overflow: TextOverflow.ellipsis, maxLines: 1),
          ),
          IconButton(onPressed: deleteFunction, icon: Icon(Icons.close)),
        ],
      ),
    );
  }
}

Widget buildSendFileButton(ThemeData theme, void Function() onPressed) {
  return TextButton(
    onPressed: onPressed,
    style: ButtonStyle(
      splashFactory: NoSplash.splashFactory,
      padding: WidgetStatePropertyAll(EdgeInsetsGeometry.all(10)),
      shape: WidgetStatePropertyAll(
        RoundedRectangleBorder(
          side: BorderSide(color: theme.colorScheme.primary, width: 4),
          borderRadius: BorderRadiusGeometry.all(Radius.circular(10)),
        ),
      ),
    ),
    child: Padding(
      padding: EdgeInsetsGeometry.all(10),
      child: Column(
        children: [
          Icon(
            Icons.file_upload_outlined,
            size: 200,
            color: theme.colorScheme.primary,
          ),
          Text("click to add a file"),
        ],
      ),
    ),
  );
}
