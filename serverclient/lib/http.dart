import 'dart:convert';
import 'dart:typed_data';

class HttpRequest {
  String? version;
  String? target;
  String? type;
  Uint8List? contents;
  final Map<String, String> headers;

  HttpRequest() : headers = {};
  HttpRequest setVersion(String? version) {
    this.version = version;
    return this;
  }

  HttpRequest setTarget(String? target) {
    this.target = target;
    return this;
  }

  HttpRequest setType(String? type) {
    this.type = type;
    return this;
  }

  HttpRequest setContents(Uint8List contents) {
    this.contents = contents;
    return this;
  }

  HttpRequest setHeader({required String name, required String value}) {
      headers[name] = value;
    return this;
  }

  Uint8List toBytes() {
    final header = StringBuffer();
    header.write("$type $target $version \r\n");
    headers.forEach((key, value) => header.write("$key: $value\r\n"));
    header.write("\r\n");
    final headerBytes = utf8.encode(header.toString());

    if (contents == null || contents!.isEmpty) {
        return Uint8List.fromList(headerBytes);
    }
    return Uint8List.fromList([...headerBytes, ...contents!]);
  }
}
