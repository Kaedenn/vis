-- This script is standalone
--
-- lua test/test_letters.lua
--

Letters = require("lua.letters")
ASCII = {}

function dump_letter(letter)
    if type(letter) == "string" then
        letter = Letters[letter]
    end
    assert(letter, "attempt to dump nil letter")

    local lines = {""}
    for idx, pixel in ipairs(letter) do
        lines[#lines] = lines[#lines] .. (pixel == 0 and ' ' or '1')
        if idx % Letters.LETTER_WIDTH == 0 then
            if idx < #letter then
                table.insert(lines, "")
            end
        end
    end
    return table.concat(lines, "\n")
end

function tohex(i) return ("0x%02x"):format(i) end

function process_character(i)
    local ch = string.char(i)
    local chu = string.char(i):upper()
    local numu = chu:byte(1)

    if i >= 0x80 then
        ch = utf8.char(i)
        chu = utf8.char(i-0x20)
        numu = i-0x20
    end

    local lu = Letters[string.char(i):upper()]
    if not lu then
        lu = Letters[utf8.char(i):upper()]
    end
    if not lu then
        lu = Letters[utf8.char(i-0x20)]
    end

    if not Letters[ch] and lu then
        local ch_str, ch_name = table.unpack(ASCII[i] or {})
        local chu_str, chu_name = table.unpack(ASCII[numu] or {})
        print(("Letter 0x%02x %s %s -> 0x%02x %s %s"):format(
            i, ch, ch_name, numu, chu_str, chu_name))
        print(dump_letter(lu))
        return true
    end

    if Letters[ch] then
        local ch_str, ch_name = utf8.char(i), "<unnamed>"
        if ASCII[i] then
            ch_str, ch_name = table.unpack(ASCII[i])
        end
        print(("Letter 0x%02x %s: %s"):format(i, ch_str, ch_name))
        print(dump_letter(Letters[ch]))
        return true
    end

    local ch_str, ch_name = utf8.char(i), "<unnamed>"
    local chu_str, chu_name = utf8.char(i-0x20), "<unnamed>"
    if ASCII[i] then
        ch_str, ch_name = table.unpack(ASCII[i])
    elseif ASCII[i-0x20] then
        ch_str, ch_name = table.unpack(ASCII[i-0x20])
    end
    return ("No entry for letter 0x%02x '%s' %s"):format(
        i, ch_str, ch_name)
end

function main()
    local notfound = {}
    for i = 0x20, 0x7f do
        local result = process_character(i)
        if type(result) == "string" then
            table.insert(notfound, result)
        end
    end
    for i = 0xa0, 0xfe do
        local result = process_character(i)
        if type(result) == "string" then
            table.insert(notfound, result)
        end
    end
    print("Entries not found:", #notfound)
    for _, line in ipairs(notfound) do
        print(line)
    end
end

ASCII = {
    [0x20] = {" ", "SPACE"},
    [0x21] = {"!", "EXCLAMATION MARK"},
    [0x22] = {"\"", "QUOTATION MARK"},
    [0x23] = {"#", "NUMBER SIGN"},
    [0x24] = {"$", "DOLLAR SIGN"},
    [0x25] = {"%", "PERCENT SIGN"},
    [0x26] = {"&", "AMPERSAND"},
    [0x27] = {"'", "APOSTROPHE"},
    [0x28] = {"(", "LEFT PARENTHESIS"},
    [0x29] = {")", "RIGHT PARENTHESIS"},
    [0x2a] = {"*", "ASTERISK"},
    [0x2b] = {"+", "PLUS SIGN"},
    [0x2c] = {",", "COMMA"},
    [0x2d] = {"-", "HYPHEN-MINUS"},
    [0x2e] = {".", "FULL STOP"},
    [0x2f] = {"/", "SOLIDUS"},
    [0x30] = {"0", "DIGIT ZERO"},
    [0x31] = {"1", "DIGIT ONE"},
    [0x32] = {"2", "DIGIT TWO"},
    [0x33] = {"3", "DIGIT THREE"},
    [0x34] = {"4", "DIGIT FOUR"},
    [0x35] = {"5", "DIGIT FIVE"},
    [0x36] = {"6", "DIGIT SIX"},
    [0x37] = {"7", "DIGIT SEVEN"},
    [0x38] = {"8", "DIGIT EIGHT"},
    [0x39] = {"9", "DIGIT NINE"},
    [0x3a] = {":", "COLON"},
    [0x3b] = {";", "SEMICOLON"},
    [0x3c] = {"<", "LESS-THAN SIGN"},
    [0x3d] = {"=", "EQUALS SIGN"},
    [0x3e] = {">", "GREATER-THAN SIGN"},
    [0x3f] = {"?", "QUESTION MARK"},
    [0x40] = {"@", "COMMERCIAL AT"},
    [0x41] = {"A", "LATIN CAPITAL LETTER A"},
    [0x42] = {"B", "LATIN CAPITAL LETTER B"},
    [0x43] = {"C", "LATIN CAPITAL LETTER C"},
    [0x44] = {"D", "LATIN CAPITAL LETTER D"},
    [0x45] = {"E", "LATIN CAPITAL LETTER E"},
    [0x46] = {"F", "LATIN CAPITAL LETTER F"},
    [0x47] = {"G", "LATIN CAPITAL LETTER G"},
    [0x48] = {"H", "LATIN CAPITAL LETTER H"},
    [0x49] = {"I", "LATIN CAPITAL LETTER I"},
    [0x4a] = {"J", "LATIN CAPITAL LETTER J"},
    [0x4b] = {"K", "LATIN CAPITAL LETTER K"},
    [0x4c] = {"L", "LATIN CAPITAL LETTER L"},
    [0x4d] = {"M", "LATIN CAPITAL LETTER M"},
    [0x4e] = {"N", "LATIN CAPITAL LETTER N"},
    [0x4f] = {"O", "LATIN CAPITAL LETTER O"},
    [0x50] = {"P", "LATIN CAPITAL LETTER P"},
    [0x51] = {"Q", "LATIN CAPITAL LETTER Q"},
    [0x52] = {"R", "LATIN CAPITAL LETTER R"},
    [0x53] = {"S", "LATIN CAPITAL LETTER S"},
    [0x54] = {"T", "LATIN CAPITAL LETTER T"},
    [0x55] = {"U", "LATIN CAPITAL LETTER U"},
    [0x56] = {"V", "LATIN CAPITAL LETTER V"},
    [0x57] = {"W", "LATIN CAPITAL LETTER W"},
    [0x58] = {"X", "LATIN CAPITAL LETTER X"},
    [0x59] = {"Y", "LATIN CAPITAL LETTER Y"},
    [0x5a] = {"Z", "LATIN CAPITAL LETTER Z"},
    [0x5b] = {"[", "LEFT SQUARE BRACKET"},
    [0x5c] = {"\\", "REVERSE SOLIDUS"},
    [0x5d] = {"]", "RIGHT SQUARE BRACKET"},
    [0x5e] = {"^", "CIRCUMFLEX ACCENT"},
    [0x5f] = {"_", "LOW LINE"},
    [0x60] = {"`", "GRAVE ACCENT"},
    [0x61] = {"a", "LATIN SMALL LETTER A"},
    [0x62] = {"b", "LATIN SMALL LETTER B"},
    [0x63] = {"c", "LATIN SMALL LETTER C"},
    [0x64] = {"d", "LATIN SMALL LETTER D"},
    [0x65] = {"e", "LATIN SMALL LETTER E"},
    [0x66] = {"f", "LATIN SMALL LETTER F"},
    [0x67] = {"g", "LATIN SMALL LETTER G"},
    [0x68] = {"h", "LATIN SMALL LETTER H"},
    [0x69] = {"i", "LATIN SMALL LETTER I"},
    [0x6a] = {"j", "LATIN SMALL LETTER J"},
    [0x6b] = {"k", "LATIN SMALL LETTER K"},
    [0x6c] = {"l", "LATIN SMALL LETTER L"},
    [0x6d] = {"m", "LATIN SMALL LETTER M"},
    [0x6e] = {"n", "LATIN SMALL LETTER N"},
    [0x6f] = {"o", "LATIN SMALL LETTER O"},
    [0x70] = {"p", "LATIN SMALL LETTER P"},
    [0x71] = {"q", "LATIN SMALL LETTER Q"},
    [0x72] = {"r", "LATIN SMALL LETTER R"},
    [0x73] = {"s", "LATIN SMALL LETTER S"},
    [0x74] = {"t", "LATIN SMALL LETTER T"},
    [0x75] = {"u", "LATIN SMALL LETTER U"},
    [0x76] = {"v", "LATIN SMALL LETTER V"},
    [0x77] = {"w", "LATIN SMALL LETTER W"},
    [0x78] = {"x", "LATIN SMALL LETTER X"},
    [0x79] = {"y", "LATIN SMALL LETTER Y"},
    [0x7a] = {"z", "LATIN SMALL LETTER Z"},
    [0x7b] = {"{", "LEFT CURLY BRACKET"},
    [0x7c] = {"|", "VERTICAL LINE"},
    [0x7d] = {"}", "RIGHT CURLY BRACKET"},
    [0x7e] = {"~", "TILDE"},
    [0xa0] = {' ', 'NO-BREAK SPACE'},
    [0xa1] = {'¡', 'INVERTED EXCLAMATION MARK'},
    [0xa2] = {'¢', 'CENT SIGN'},
    [0xa3] = {'£', 'POUND SIGN'},
    [0xa4] = {'¤', 'CURRENCY SIGN'},
    [0xa5] = {'¥', 'YEN SIGN'},
    [0xa6] = {'¦', 'BROKEN BAR'},
    [0xa7] = {'§', 'SECTION SIGN'},
    [0xa8] = {'¨', 'DIAERESIS'},
    [0xa9] = {'©', 'COPYRIGHT SIGN'},
    [0xaa] = {'ª', 'FEMININE ORDINAL INDICATOR'},
    [0xab] = {'«', 'LEFT-POINTING DOUBLE ANGLE QUOTATION MARK'},
    [0xac] = {'¬', 'NOT SIGN'},
    [0xad] = {'­', 'SOFT HYPHEN'},
    [0xae] = {'®', 'REGISTERED SIGN'},
    [0xaf] = {'¯', 'MACRON'},
    [0xb0] = {'°', 'DEGREE SIGN'},
    [0xb1] = {'±', 'PLUS-MINUS SIGN'},
    [0xb2] = {'²', 'SUPERSCRIPT TWO'},
    [0xb3] = {'³', 'SUPERSCRIPT THREE'},
    [0xb4] = {'´', 'ACUTE ACCENT'},
    [0xb5] = {'µ', 'MICRO SIGN'},
    [0xb6] = {'¶', 'PILCROW SIGN'},
    [0xb7] = {'·', 'MIDDLE DOT'},
    [0xb8] = {'¸', 'CEDILLA'},
    [0xb9] = {'¹', 'SUPERSCRIPT ONE'},
    [0xba] = {'º', 'MASCULINE ORDINAL INDICATOR'},
    [0xbb] = {'»', 'RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK'},
    [0xbc] = {'¼', 'VULGAR FRACTION ONE QUARTER'},
    [0xbd] = {'½', 'VULGAR FRACTION ONE HALF'},
    [0xbe] = {'¾', 'VULGAR FRACTION THREE QUARTERS'},
    [0xbf] = {'¿', 'INVERTED QUESTION MARK'},
    [0xc0] = {'À', 'LATIN CAPITAL LETTER A WITH GRAVE'},
    [0xc1] = {'Á', 'LATIN CAPITAL LETTER A WITH ACUTE'},
    [0xc2] = {'Â', 'LATIN CAPITAL LETTER A WITH CIRCUMFLEX'},
    [0xc3] = {'Ã', 'LATIN CAPITAL LETTER A WITH TILDE'},
    [0xc4] = {'Ä', 'LATIN CAPITAL LETTER A WITH DIAERESIS'},
    [0xc5] = {'Å', 'LATIN CAPITAL LETTER A WITH RING'},
    [0xc6] = {'Æ', 'LATIN CAPITAL LETTER AE'},
    [0xc7] = {'Ç', 'LATIN CAPITAL LETTER C WITH CEDILLA'},
    [0xc8] = {'È', 'LATIN CAPITAL LETTER E WITH GRAVE'},
    [0xc9] = {'É', 'LATIN CAPITAL LETTER E WITH ACUTE'},
    [0xca] = {'Ê', 'LATIN CAPITAL LETTER E WITH CIRCUMFLEX'},
    [0xcb] = {'Ë', 'LATIN CAPITAL LETTER E WITH DIAERESIS'},
    [0xcc] = {'Ì', 'LATIN CAPITAL LETTER I WITH GRAVE'},
    [0xcd] = {'Í', 'LATIN CAPITAL LETTER I WITH ACUTE'},
    [0xce] = {'Î', 'LATIN CAPITAL LETTER I WITH CIRCUMFLEX'},
    [0xcf] = {'Ï', 'LATIN CAPITAL LETTER I WITH DIAERESIS'},
    [0xd0] = {'Ð', 'LATIN CAPITAL LETTER ETH'},
    [0xd1] = {'Ñ', 'LATIN CAPITAL LETTER N WITH TILDE'},
    [0xd2] = {'Ò', 'LATIN CAPITAL LETTER O WITH GRAVE'},
    [0xd3] = {'Ó', 'LATIN CAPITAL LETTER O WITH ACUTE'},
    [0xd4] = {'Ô', 'LATIN CAPITAL LETTER O WITH CIRCUMFLEX'},
    [0xd5] = {'Õ', 'LATIN CAPITAL LETTER O WITH TILDE'},
    [0xd6] = {'Ö', 'LATIN CAPITAL LETTER O WITH DIAERESIS'},
    [0xd7] = {'×', 'MULTIPLICATION SIGN'},
    [0xd8] = {'Ø', 'LATIN CAPITAL LETTER O WITH STROKE'},
    [0xd9] = {'Ù', 'LATIN CAPITAL LETTER U WITH GRAVE'},
    [0xda] = {'Ú', 'LATIN CAPITAL LETTER U WITH ACUTE'},
    [0xdb] = {'Û', 'LATIN CAPITAL LETTER U WITH CIRCUMFLEX'},
    [0xdc] = {'Ü', 'LATIN CAPITAL LETTER U WITH DIAERESIS'},
    [0xdd] = {'Ý', 'LATIN CAPITAL LETTER Y WITH ACUTE'},
    [0xde] = {'Þ', 'LATIN CAPITAL LETTER THORN'},
    [0xdf] = {'ß', 'LATIN SMALL LETTER SHARP S'},
    [0xe6] = {'æ', 'LATIN SMALL LETTER AE'},
    [0xf0] = {'ð', 'LATIN SMALL LETTER ETH'},
    [0xf7] = {'÷', 'DIVISION SIGN'},
    [0xfe] = {'þ', 'LATIN SMALL LETTER THORN'},
}

main()
