class BilingualString {
    final String english;
    final String french;
    const BilingualString(this.english, this.french);
    String get(bool isEnglish) {
        return isEnglish ? english : french;
    }
}
