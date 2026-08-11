import 'package:flutter/material.dart';
import 'package:serverclient/main.dart';

Widget serverInfoPage(HomePageState state) {
  return ElevatedButton(
    onPressed: () {
      state.pipe.httpGET("/hierarchy");
    },
    child: Text("Ping"),
  );
}
