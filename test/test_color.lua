Harness = require('harness')
Vis = require("Vis")
VisUtil = require("visutil")

local function assert_approx(a, b, eps, msg)
    if math.abs(a - b) > eps then
        error(string.format("Assertion failed: %s (Expected %f, got %f)", msg, b, a))
    end
end

local EPS = 1e-4

-- Test RGB to HSL
local function test_rgb2hsl()
    local h, s, l
    h, s, l = VisUtil.rgb2hsl(1, 0, 0)
    assert_approx(h, 0, EPS, "Red H")
    assert_approx(s, 1, EPS, "Red S")
    assert_approx(l, 0.5, EPS, "Red L")

    h, s, l = VisUtil.rgb2hsl(0, 1, 0)
    assert_approx(h, 120, EPS, "Green H")
    assert_approx(s, 1, EPS, "Green S")
    assert_approx(l, 0.5, EPS, "Green L")

    h, s, l = VisUtil.rgb2hsl(0, 0, 1)
    assert_approx(h, 240, EPS, "Blue H")
    assert_approx(s, 1, EPS, "Blue S")
    assert_approx(l, 0.5, EPS, "Blue L")

    h, s, l = VisUtil.rgb2hsl(1, 1, 1)
    assert_approx(l, 1, EPS, "White L")
    assert_approx(s, 0, EPS, "White S")

    h, s, l = VisUtil.rgb2hsl(0, 0, 0)
    assert_approx(l, 0, EPS, "Black L")
    assert_approx(s, 0, EPS, "Black S")
end

-- Test HSL to RGB
local function test_hsl2rgb()
    local r, g, b
    r, g, b = VisUtil.hsl2rgb(0, 1, 0.5)
    assert_approx(r, 1, EPS, "Red R from HSL")
    assert_approx(g, 0, EPS, "Red G from HSL")
    assert_approx(b, 0, EPS, "Red B from HSL")

    r, g, b = VisUtil.hsl2rgb(120, 1, 0.5)
    assert_approx(r, 0, EPS, "Green R from HSL")
    assert_approx(g, 1, EPS, "Green G from HSL")
    assert_approx(b, 0, EPS, "Green B from HSL")

    r, g, b = VisUtil.hsl2rgb(240, 1, 0.5)
    assert_approx(r, 0, EPS, "Blue R from HSL")
    assert_approx(g, 0, EPS, "Blue G from HSL")
    assert_approx(b, 1, EPS, "Blue B from HSL")
end

-- Test RGB to HSV
local function test_rgb2hsv()
    local h, s, v
    h, s, v = VisUtil.rgb2hsv(1, 0, 0)
    assert_approx(h, 0, EPS, "Red H")
    assert_approx(s, 1, EPS, "Red S")
    assert_approx(v, 1, EPS, "Red V")

    h, s, v = VisUtil.rgb2hsv(0, 1, 0)
    assert_approx(h, 120, EPS, "Green H")
    assert_approx(s, 1, EPS, "Green S")
    assert_approx(v, 1, EPS, "Green V")

    h, s, v = VisUtil.rgb2hsv(0, 0, 1)
    assert_approx(h, 240, EPS, "Blue H")
    assert_approx(s, 1, EPS, "Blue S")
    assert_approx(v, 1, EPS, "Blue V")

    h, s, v = VisUtil.rgb2hsv(1, 1, 1)
    assert_approx(v, 1, EPS, "White V")
    assert_approx(s, 0, EPS, "White S")
end

-- Test HSV to RGB
local function test_hsv2rgb()
    local r, g, b
    r, g, b = VisUtil.hsv2rgb(0, 1, 1)
    assert_approx(r, 1, EPS, "Red R from HSV")
    assert_approx(g, 0, EPS, "Red G from HSV")
    assert_approx(b, 0, EPS, "Red B from HSV")

    r, g, b = VisUtil.hsv2rgb(120, 1, 1)
    assert_approx(r, 0, EPS, "Green R from HSV")
    assert_approx(g, 1, EPS, "Green G from HSV")
    assert_approx(b, 0, EPS, "Green B from HSV")

    r, g, b = VisUtil.hsv2rgb(240, 1, 1)
    assert_approx(r, 0, EPS, "Blue R from HSV")
    assert_approx(g, 0, EPS, "Blue G from HSV")
    assert_approx(b, 1, EPS, "Blue B from HSV")
end

-- Test RGB to Oklab
local function test_rgb2oklab()
    local L, a, b
    L, a, b = VisUtil.rgb2oklab(1, 1, 1)
    assert_approx(L, 1, EPS, "White L")
    assert_approx(a, 0, EPS, "White a")
    assert_approx(b, 0, EPS, "White b")

    L, a, b = VisUtil.rgb2oklab(0, 0, 0)
    assert_approx(L, 0, EPS, "Black L")
    assert_approx(a, 0, EPS, "Black a")
    assert_approx(b, 0, EPS, "Black b")

    L, a, b = VisUtil.rgb2oklab(1, 0, 0)
    assert_approx(L, 0.627955, EPS, "Red L")
    assert_approx(a, 0.224863, EPS, "Red a")
    assert_approx(b, 0.125846, EPS, "Red b")

    L, a, b = VisUtil.rgb2oklab(0, 1, 0)
    assert_approx(L, 0.86644, EPS, "Green L")
    assert_approx(a, -0.233887, EPS, "Green a")
    assert_approx(b, 0.179498, EPS, "Green b")

    L, a, b = VisUtil.rgb2oklab(0, 0, 1)
    assert_approx(L, 0.452014, EPS, "Blue L")
    assert_approx(a, -0.032457, EPS, "Blue a")
    assert_approx(b, -0.311528, EPS, "Blue b")
end

local function test_roundtrips()
    local rgbs = {
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
        {1, 1, 1}, {0, 0, 0}, {0.5, 0.5, 0.5},
        {0.2, 0.8, 0.4}, {0.9, 0.1, 0.7}
    }

    for _, color in ipairs(rgbs) do
        local r, g, b = color[1], color[2], color[3]

        -- HSL Roundtrip
        local h, s, l = VisUtil.rgb2hsl(r, g, b)
        local rt_r, rt_g, rt_b = VisUtil.hsl2rgb(h, s, l)
        assert_approx(r, rt_r, EPS, "HSL Roundtrip R")
        assert_approx(g, rt_g, EPS, "HSL Roundtrip G")
        assert_approx(b, rt_b, EPS, "HSL Roundtrip B")

        -- HSV Roundtrip
        local h2, s2, v = VisUtil.rgb2hsv(r, g, b)
        local rt2_r, rt2_g, rt2_b = VisUtil.hsv2rgb(h2, s2, v)
        assert_approx(r, rt2_r, EPS, "HSV Roundtrip R")
        assert_approx(g, rt2_g, EPS, "HSV Roundtrip G")
        assert_approx(b, rt2_b, EPS, "HSV Roundtrip B")

        -- Oklab Roundtrip
        local L, a, o_b = VisUtil.rgb2oklab(r, g, b)
        local rt3_r, rt3_g, rt3_b = VisUtil.oklab2rgb(L, a, o_b)
        assert_approx(r, rt3_r, EPS, "Oklab Roundtrip R")
        assert_approx(g, rt3_g, EPS, "Oklab Roundtrip G")
        assert_approx(b, rt3_b, EPS, "Oklab Roundtrip B")
    end
end

local function test_blend()
    local c1 = {1, 0, 0}
    local c2 = {0, 0, 1}
    
    local b0 = VisUtil.blend_rgb(c1, c2, 0)
    assert_approx(b0[1], 1, EPS, "Blend 0 R")
    assert_approx(b0[2], 0, EPS, "Blend 0 G")
    assert_approx(b0[3], 0, EPS, "Blend 0 B")

    local b1 = VisUtil.blend_rgb(c1, c2, 1)
    assert_approx(b1[1], 0, EPS, "Blend 1 R")
    assert_approx(b1[2], 0, EPS, "Blend 1 G")
    assert_approx(b1[3], 1, EPS, "Blend 1 B")
    
    local b05 = VisUtil.blend_rgb(c1, c2, 0.5)
    assert(b05[1] > 0.0, "Blend 0.5 R")
    assert(b05[3] > 0.0, "Blend 0.5 B")
end

-- Run all tests
test_rgb2hsl()
test_hsl2rgb()
test_rgb2hsv()
test_hsv2rgb()
test_rgb2oklab()
test_roundtrips()
test_blend()

print("All color conversion tests passed successfully!")
Vis.exit(Vis.flist, 0)
