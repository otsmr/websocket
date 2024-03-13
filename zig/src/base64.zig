//
// Copyright (c) 2024, Tobias Müller <git@tsmr.eu>
//

const std = @import("std");

const EncodeError = error{ WrongLength, OutputBufferToSmall };

const standard_alphabet_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

fn calc_size(input_len: usize) usize {
    const leftover = input_len % 3;
    return @divTrunc(input_len, 3) * 4 + @divTrunc(leftover * 4 + 2, 3);
}

/// Encodes the given input with base64. The standard alphabet is used.
pub fn encode(input: []const u8, output: []u8) EncodeError![]const u8 {
    if (calc_size(input.len) > output.len) {
        return EncodeError.OutputBufferToSmall;
    }

    var pointer: usize = 0;
    var offset: usize = 0;

    var tmp: [3]u8 = undefined;

    while (input.len > pointer) {
        for (0..3) |i| {
            if (input.len <= pointer + i) {
                tmp[i] = 0x00;
            } else {
                tmp[i] = input[pointer + i];
            }
        }

        if (output.len < offset + 3) {
            return EncodeError.OutputBufferToSmall;
        }

        output[offset + 0] = standard_alphabet_chars[tmp[0] >> 2 & 63];
        output[offset + 1] = standard_alphabet_chars[((tmp[0] & 3) << 4 | tmp[1] >> 4) & 63];
        output[offset + 2] = standard_alphabet_chars[((tmp[1] & 15) << 2 | tmp[2] >> 6) & 63];
        output[offset + 3] = standard_alphabet_chars[tmp[2] & 63];

        pointer += 3;
        offset += 4;
    }

    var i: usize = 0;
    while (i < (3 - (input.len % 3)) % 3) : (i += 1) {
        output[offset - i - 1] = '=';
    }

    return output[0..offset];
}

test "String copy" {
    const test_strings = [_][2][]const u8{
        .{ "", "" },
        .{ "f", "Zg==" },
        .{ "fo", "Zm8=" },
        .{ "foo", "Zm9v" },
        .{ "foob", "Zm9vYg==" },
        .{ "fooba", "Zm9vYmE=" },
        .{ "foobar", "Zm9vYmFy" },
    };

    var buffer: [100:0]u8 = undefined;

    for (test_strings) |test_string| {
        const output = try encode(test_string[0], buffer[0..100]);
        try std.testing.expect(std.mem.eql(u8, output, test_string[1]));
    }
}
