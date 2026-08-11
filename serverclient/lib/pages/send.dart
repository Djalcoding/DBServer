import 'package:flutter/material.dart';
import 'package:serverclient/filemanagement.dart';
import 'package:serverclient/main.dart';

Widget _buildFileList(HomePageState state, {int maxLength = 10}) {
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
                      color: theme
                          .colorScheme
                          .secondaryFixed, // TODO : darker text
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

Widget sendFilesPage(HomePageState state) {
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
          Expanded(child: _buildFileList(state)),
        ],
      );
    },
  );
}
