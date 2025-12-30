math = require("math")
utf8 = require("utf8")

Letters = {}
Letters.LETTER_WIDTH = 5
Letters.LETTER_HEIGHT = 7
Letters.LETTER_SPACING = 1

-- Determine the width and height of a given string
function Letters.find_extents(s)
    local width = 0
    local height = Letters.LETTER_HEIGHT
    local line_width = 0
    for _, c in utf8.codes(s) do
        if c == string.byte("\n") then
            width = math.max(line_width, width)
            line_width = 0
            height = height + Letters.LETTER_HEIGHT + Letters.LETTER_SPACING
        else
            line_width = line_width + Letters.LETTER_WIDTH + Letters.LETTER_SPACING
        end
    end
    width = math.max(line_width, width)
    return width - 1, height
end

-- Call func(x, y) for each "on pixel" in the given letter
function Letters.map_fn_xy(letter, func)
    for idx, bit in ipairs(Letters[letter] or Letters['?']) do
        local lx = (idx - 1) % Letters.LETTER_WIDTH + 1
        local ly = math.floor((idx - 1) / Letters.LETTER_WIDTH) + 1
        if bit == 1 then
            func(lx, ly)
        end
    end
end

function Letters.combine(base_letter, combiner)
    local result = {}
    for _, row in ipairs(combiner) do
        table.insert(result, row)
    end
    for _, row in ipairs(base_letter) do
        table.insert(result, row)
    end
    return result
end

Combiners = {
    ["GRAVE"] = {
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0
    },
    ["ACUTE"] = {
        0, 0, 0, 1, 0,
        0, 0, 1, 0, 0
    },
    ["CIRCUMFLEX"] = {
        0, 0, 1, 0, 0,
        0, 1, 0, 1, 0
    },
    ["TILDE"] = {
        0, 0, 1, 0, 1,
        0, 1, 0, 1, 0
    },
    ["DIAERESIS"] = {
        0, 1, 0, 1, 0,
        0, 0, 0, 0, 0
    },
    ["RING ABOVE"] = {
        0, 0, 1, 0, 0,
        0, 1, 0, 1, 0,
        0, 0, 1, 0, 0
    }
}

Letters["'"] = {
    0, 0, 0, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
}

Letters['0'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['1'] = {
    0, 0, 1, 0, 0,
    0, 1, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
}

Letters['2'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    1, 1, 1, 1, 1,
}

Letters['3'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
    0, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['4'] = {
    0, 0, 0, 1, 0,
    0, 0, 1, 1, 0,
    0, 1, 0, 1, 0,
    1, 0, 0, 1, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 1, 0,
    0, 0, 0, 1, 0,
}

Letters['5'] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['6'] = {
    0, 0, 1, 1, 0,
    0, 1, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0
}

Letters['7'] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    0, 1, 0, 0, 0,
    1, 0, 0, 0, 0,
}

Letters['8'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['9'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 1, 1, 0, 0,
}

Letters[' '] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
}

Letters['!'] = {
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 1, 0, 0,
}

Letters['.'] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 1, 1, 0, 0,
    0, 1, 1, 0, 0,
}

Letters[','] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 1, 1, 0, 0,
    0, 1, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
}

Letters['?'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 1, 0, 0,
}

Letters['-'] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
}

Letters['+'] = {
    0, 0, 0, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 0, 0, 0,
}

Letters['_'] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
}

Letters['='] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
}

Letters['A'] = {
    0, 0, 1, 0, 0,
    0, 1, 0, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
}

Letters['B'] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
}

Letters['C'] = {
    0, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    0, 1, 1, 1, 1,
}

Letters['D'] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
}

Letters['E'] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
}

Letters['F'] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
}

Letters['G'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 1, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['H'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
}

Letters['I'] = {
    1, 1, 1, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    1, 1, 1, 1, 1,
}

Letters['J'] = {
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['K'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 1, 0,
    1, 1, 1, 0, 0,
    1, 0, 0, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
}

Letters['L'] = {
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
}

Letters['M'] = {
    1, 0, 0, 0, 1,
    1, 1, 0, 1, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
}

Letters['N'] = {
    1, 0, 0, 0, 1,
    1, 1, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 1, 1,
    1, 0, 0, 0, 1,
}

Letters['O'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['P'] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
}

Letters['Q'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 1, 0,
    0, 1, 1, 0, 1,
}

Letters['R'] = {
    1, 1, 1, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 0,
    1, 0, 1, 0, 0,
    1, 0, 0, 1, 0,
    1, 0, 0, 0, 1,
}

Letters['S'] = {
    0, 1, 1, 1, 0,
    1, 0, 0, 0, 0,
    1, 0, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 0, 1,
    0, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['T'] = {
    1, 1, 1, 1, 1,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
}

Letters['U'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 1, 1, 0,
}

Letters['V'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 0, 1, 0,
    0, 1, 0, 1, 0,
    0, 0, 1, 0, 0,
}

Letters['W'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 1, 0, 1, 1,
    1, 0, 0, 0, 1,
}

Letters['X'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 1, 0,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
}

Letters['Y'] = {
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    0, 1, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
}

Letters['Z'] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
}

-- 0xa0   NO-BREAK SPACE
Letters['\xa0'] = Letters[' ']

-- 0xa1 ¡ INVERTED EXCLAMATION MARK
-- 0xa2 ¢ CENT SIGN
-- 0xa3 £ POUND SIGN
-- 0xa4 ¤ CURRENCY SIGN
-- 0xa5 ¥ YEN SIGN
-- 0xa6 ¦ BROKEN BAR
-- 0xa7 § SECTION SIGN
-- 0xa8 ¨ DIAERESIS
-- 0xa9 © COPYRIGHT SIGN
-- 0xaa ª FEMININE ORDINAL INDICATOR
-- 0xab « LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
-- 0xac ¬ NOT SIGN
-- 0xad ­ SOFT HYPHEN
-- 0xae ® REGISTERED SIGN
-- 0xaf ¯ MACRON
-- 0xb0 ° DEGREE SIGN
-- 0xb1 ± PLUS-MINUS SIGN
-- 0xb2 ² SUPERSCRIPT TWO
-- 0xb3 ³ SUPERSCRIPT THREE
-- 0xb4 ´ ACUTE ACCENT
-- 0xb5 µ MICRO SIGN
-- 0xb6 ¶ PILCROW SIGN
-- 0xb7 · MIDDLE DOT
-- 0xb8 ¸ CEDILLA
-- 0xb9 ¹ SUPERSCRIPT ONE
-- 0xba º MASCULINE ORDINAL INDICATOR
-- 0xbb » RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
-- 0xbc ¼ VULGAR FRACTION ONE QUARTER
-- 0xbd ½ VULGAR FRACTION ONE HALF
-- 0xbe ¾ VULGAR FRACTION THREE QUARTERS
-- 0xbf ¿ INVERTED QUESTION MARK

-- 0xc0 À LATIN CAPITAL LETTER A WITH GRAVE
Letters['\xc0'] = Letters.combine(Letters['A'], Combiners["GRAVE"])

-- 0xc1 Á LATIN CAPITAL LETTER A WITH ACUTE
Letters['\xc1'] = Letters.combine(Letters['A'], Combiners["ACUTE"])

-- 0xc2 Â LATIN CAPITAL LETTER A WITH CIRCUMFLEX
Letters['\xc2'] = Letters.combine(Letters['A'], Combiners["CIRCUMFLEX"])
-- 0xc3 Ã LATIN CAPITAL LETTER A WITH TILDE
Letters['\xc3'] = Letters.combine(Letters['A'], Combiners["TILDE"])
-- 0xc4 Ä LATIN CAPITAL LETTER A WITH DIAERESIS
Letters['\xc4'] = Letters.combine(Letters['A'], Combiners["DIAERESIS"])
-- 0xc5 Å LATIN CAPITAL LETTER A WITH RING
Letters['\xc5'] = Letters.combine(Letters['A'], Combiners["RING ABOVE"])
-- 0xc6 Æ LATIN CAPITAL LETTER AE
-- 0xc7 Ç LATIN CAPITAL LETTER C WITH CEDILLA
-- 0xc8 È LATIN CAPITAL LETTER E WITH GRAVE
Letters['\xc8'] = Letters.combine(Letters['E'], Combiners["GRAVE"])
-- 0xc9 É LATIN CAPITAL LETTER E WITH ACUTE
Letters['\xc9'] = Letters.combine(Letters['E'], Combiners["ACUTE"])
-- 0xca Ê LATIN CAPITAL LETTER E WITH CIRCUMFLEX
Letters['\xca'] = Letters.combine(Letters['E'], Combiners["CIRCUMFLEX"])
-- 0xcb Ë LATIN CAPITAL LETTER E WITH DIAERESIS
Letters['\xcb'] = Letters.combine(Letters['E'], Combiners["DIAERESIS"])
-- 0xcc Ì LATIN CAPITAL LETTER I WITH GRAVE
Letters['\xcc'] = Letters.combine(Letters['I'], Combiners["GRAVE"])
-- 0xcd Í LATIN CAPITAL LETTER I WITH ACUTE
Letters['\xcd'] = Letters.combine(Letters['I'], Combiners["ACUTE"])
-- 0xce Î LATIN CAPITAL LETTER I WITH CIRCUMFLEX
Letters['\xce'] = Letters.combine(Letters['I'], Combiners["CIRCUMFLEX"])
-- 0xcf Ï LATIN CAPITAL LETTER I WITH DIAERESIS
Letters['\xcf'] = Letters.combine(Letters['I'], Combiners["DIAERESIS"])
-- 0xd0 Ð LATIN CAPITAL LETTER ETH
-- 0xd1 Ñ LATIN CAPITAL LETTER N WITH TILDE
Letters['\xd1'] = Letters.combine(Letters['N'], Combiners["TILDE"])
-- 0xd2 Ò LATIN CAPITAL LETTER O WITH GRAVE
Letters['\xd2'] = Letters.combine(Letters['O'], Combiners["GRAVE"])
-- 0xd3 Ó LATIN CAPITAL LETTER O WITH ACUTE
Letters['\xd3'] = Letters.combine(Letters['O'], Combiners["ACUTE"])
-- 0xd4 Ô LATIN CAPITAL LETTER O WITH CIRCUMFLEX
Letters['\xd4'] = Letters.combine(Letters['O'], Combiners["CIRCUMFLEX"])
-- 0xd5 Õ LATIN CAPITAL LETTER O WITH TILDE
Letters['\xd5'] = Letters.combine(Letters['O'], Combiners["TILDE"])
-- 0xd6 Ö LATIN CAPITAL LETTER O WITH DIAERESIS
Letters['\xd6'] = Letters.combine(Letters['O'], Combiners["DIAERESIS"])
-- 0xd7 × MULTIPLICATION SIGN
Letters['\xd7'] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 1, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 1, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
}

-- 0xd8 Ø LATIN CAPITAL LETTER O WITH STROKE
Letters['\xd8'] = {
    0, 1, 1, 1, 1,
    1, 0, 0, 1, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 1, 0, 1,
    1, 1, 0, 0, 1,
    1, 1, 1, 1, 0,
}
-- 0xd9 Ù LATIN CAPITAL LETTER U WITH GRAVE
Letters['\xd9'] = Letters.combine(Letters['U'], Combiners["GRAVE"])
-- 0xda Ú LATIN CAPITAL LETTER U WITH ACUTE
Letters['\xda'] = Letters.combine(Letters['U'], Combiners["ACUTE"])
-- 0xdb Û LATIN CAPITAL LETTER U WITH CIRCUMFLEX
Letters['\xdb'] = Letters.combine(Letters['U'], Combiners["CIRCUMFLEX"])
-- 0xdc Ü LATIN CAPITAL LETTER U WITH DIAERESIS
Letters['\xdc'] = Letters.combine(Letters['U'], Combiners["DIAERESIS"])
-- 0xdd Ý LATIN CAPITAL LETTER Y WITH ACUTE
Letters['\xdd'] = Letters.combine(Letters['Y'], Combiners["ACUTE"])
-- 0xde Þ LATIN CAPITAL LETTER THORN
-- 0xdf ß LATIN SMALL LETTER SHARP S
-- 0xe6 æ LATIN SMALL LETTER AE
-- 0xf0 ð LATIN SMALL LETTER ETH
-- 0xf7 ÷ DIVISION SIGN
-- 0xfe þ LATIN SMALL LETTER THORN

-- Map the remaining lowercase Latin-1 characters, as we only support uppercase
local function map_lcase_latin1()
    for i = 0xe0, 0xfd do
        local lchar = string.char(i)
        local uchar = string.char(i-0x20)
        if Letters[uchar] then
            Letters[lchar] = Letters[uchar]
        end
    end
end
map_lcase_latin1()

return Letters
