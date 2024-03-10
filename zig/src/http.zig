const std = @import("std");

const HttpHeader = struct { status_code: ?usize = null, fields: std.StringHashMap([]const u8) };

const HttpParserState = enum {
    Method,
    Resource,
    Protocol,
    HeaderName,
    HeaderValue,
    Body,
};

pub fn parse_http_header(allocator: std.mem.Allocator, raw_data: []u8) !HttpHeader {
    var http_fields = std.StringHashMap([]const u8).init(allocator);

    var state: HttpParserState = .Method;
    var start: usize = 0;

    for (raw_data, 0..) |byte, end| {
        switch (state) {
            .Method => {
                if (byte == ' ') {
                    if (!std.mem.eql(u8, raw_data[start..end], "GET")) {
                        return error.IllegalMethode;
                    }
                    start = end;
                    state = .Resource;
                }
            },
            .Resource => {},
            .Protocol => {},
            .HeaderName => {},
            .HeaderValue => {},
            .Body => {},
        }
    }

    if (state != .Body) {
        return error.IllegalHttpRequest;
    }

    return HttpHeader{ .fields = http_fields };
}
