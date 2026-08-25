import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:serverclient/main.dart';

Widget connectPage(HomePageState state) {
  return Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text(
        "Connect",
        style: TextStyle(fontSize: 50, fontWeight: FontWeight.bold),
      ),
      SizedBox(height: 30),
      buildHostInput(state),
      Expanded(child: SizedBox()),
      Align(alignment: Alignment.bottomCenter, child: connectButton(state)),
    ],
  );
}

Widget buildHostInput(HomePageState state, {double spacing = 20}) {
  return Row(
    children: [
      SizedBox(width: spacing),
      Expanded(
        flex: 2,
        child: TextFormField(
          initialValue: state.targetHost,
          decoration: rectangleDecoration(labelText: "IP address"),
          onChanged: (text) {
            state.targetHost = text;
          },
          maxLength: 15,
          inputFormatters: [
            FilteringTextInputFormatter.allow(RegExp(r'[0-9.]')),
          ],
        ),
      ),
      SizedBox(width: spacing),
      Expanded(
        flex: 1,
        child: TextFormField(
          initialValue: state.targetPort.toString(),
          decoration: rectangleDecoration(labelText: "Port"),
          onChanged: (text) => state.targetPort = int.parse(text),
          maxLength: 4,
          inputFormatters: [FilteringTextInputFormatter.digitsOnly],
        ),
      ),
      SizedBox(width: spacing),
    ],
  );
}

Widget connectButton(HomePageState state) {
  return ElevatedButton(
    style: ButtonStyle(
      shape: WidgetStatePropertyAll(
        RoundedRectangleBorder(
          borderRadius: BorderRadiusGeometry.all(Radius.circular(5)),
        ),
      ),
    ),
    onPressed: () {
      state.pipe.connect(host: state.targetHost, port: state.targetPort);
    },
    child: Text("Connect"),
  );
}

InputDecoration rectangleDecoration({String? labelText, String? hintText}) {
  return InputDecoration(
    hintText: hintText,
    labelText: labelText,
    border: OutlineInputBorder(
      borderSide: BorderSide(),
      borderRadius: BorderRadius.all(Radius.circular(8)),
    ),
  );
}
