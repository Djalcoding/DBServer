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
      _buildUsernameField(),
      _buildPasswordFields(passwordCount: 12),
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

Widget _buildUsernameField() {
  return Padding(
    padding: .symmetric(vertical: 20, horizontal: 20),
    child: TextFormField(
      decoration: rectangleDecoration(labelText: "Username"),
      inputFormatters: [FilteringTextInputFormatter.deny(RegExp(r'\*| '))],
      maxLength: 25,
    ),
  );
}

class PasswordField extends StatelessWidget {
  final FocusNode? next;
  final FocusNode curr;
  final FocusNode? prev;
  final int id;
  const PasswordField({
    super.key,
    required this.id,
    required this.next,
    required this.curr,
    required this.prev,
  });
  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 165,
      child: TextFormField(
        focusNode: curr,
        onFieldSubmitted: (String s) {
          next?.requestFocus();
        },
        onChanged: (String s) {
          if (s.isEmpty) {
            prev?.requestFocus();
          }
        },
        maxLength: 12,
        decoration: rectangleDecoration(labelText: "Key #$id", counterText: ""),
      ),
    );
  }
}

Widget _buildPasswordFields({int passwordCount = 12}) {
  List<FocusNode> nodes = [];
  for (int i = 0; i < passwordCount; i++) {
    nodes.add(FocusNode());
  }
  List<Widget> fields = [];
  for (int i = 0; i < passwordCount; i++) {
    fields.add(
      SizedBox(
        width: 165,
        child: PasswordField(
          id: i + 1,
          prev: i == 0 ? null : nodes[i - 1],
          curr: nodes[i],
          next: i == passwordCount - 1 ? null : nodes[i + 1],
        ),
      ),
    );
  }

  return Padding(
    padding: .symmetric(horizontal: 20),
    child: Center(
      child: Wrap(
        alignment: .center,
        spacing: 20,
        runSpacing: 15,
        children: fields,
      ),
    ),
  );
}

Widget connectButton(HomePageState state) {
  return Builder(
    builder: (BuildContext context) {
      ThemeData theme = Theme.of(context);
      return SizedBox(
        width: 300,
        child: FittedBox(
          child: ElevatedButton(
            style: ButtonStyle(
              elevation: .all(30),
              backgroundColor: .fromMap({
                WidgetState.pressed: Color.fromRGBO(0, 50, 175, 1),
                WidgetState.hovered: Color.fromRGBO(0, 65, 175, 1),
                WidgetState.any: Color.fromRGBO(0, 85, 175, 1),
              }),
              shadowColor: .all(theme.colorScheme.shadow),
              splashFactory: NoSplash.splashFactory,
              shape: WidgetStatePropertyAll(
                RoundedRectangleBorder(borderRadius: .all(Radius.circular(5))),
              ),
            ),
            onPressed: () {
              state.pipe.connect(
                host: state.targetHost,
                port: state.targetPort,
              );
            },
            child: Text("Connect", style: TextStyle(color: Colors.white)),
          ),
        ),
      );
    },
  );
}

InputDecoration rectangleDecoration({
  String? labelText,
  String? hintText,
  String? counterText,
}) {
  return InputDecoration(
    hintText: hintText,
    labelText: labelText,
    counterText: counterText,
    border: OutlineInputBorder(
      borderSide: BorderSide(),
      borderRadius: BorderRadius.all(Radius.circular(8)),
    ),
  );
}
