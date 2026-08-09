import 'package:flutter/material.dart';

const String appVersion = "1.0.0";
final ThemeData lightTheme = ThemeData(
  brightness: Brightness.light,
  colorScheme: ColorScheme.fromSeed(seedColor: Colors.blueAccent),
  textTheme: TextTheme(),
);
final ThemeData darkTheme = ThemeData(
  brightness: Brightness.dark,
  colorScheme: ColorScheme.fromSeed(seedColor: Colors.blueAccent, brightness: Brightness.dark),
  textTheme: TextTheme(),
);
