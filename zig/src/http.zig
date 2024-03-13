const std = @import("std");
const eql = std.mem.eql;

const HttpHeader = struct {
    status_code: ?usize = null,
    resource: []u8 = undefined,
    fields: std.StringHashMap([]const u8),
    body: []u8 = undefined,

    fn status_code_as_string(self: *HttpHeader) ![]const u8 {
        if (self.status_code) |code| {
            return switch (code) {
                101 => "Switching Protocols",
                400 => "Bad Request",
                else => "Unkown Error",
            };
        } else {
            return error.NoStatusCodeGiven;
        }
    }

    pub fn to_str(self: *HttpHeader, buffer: []u8) ![]u8 {
        var status_code: usize = 200;
        if (self.status_code) |sc| {
            status_code = sc;
        }
        var buffer_len: usize = 0;
        var str = try std.fmt.bufPrint(buffer, "HTTP/1.1 {d} {s}\r\n", .{ status_code, try self.status_code_as_string() });
        buffer_len = str.len;

        var iter = self.fields.iterator();

        while (iter.next()) |entry| {
            str = try std.fmt.bufPrint(buffer[buffer_len..], "{s}: {s}\r\n", .{ entry.key_ptr.*, entry.value_ptr.* });
            buffer_len += str.len;
        }

        str = try std.fmt.bufPrint(buffer[buffer_len..], "\r\n", .{});
        return buffer[0 .. buffer_len + 2];
    }
};

const HttpParserState = enum {
    Method,
    Resource,
    Protocol,
    HeaderName,
    HeaderValue,
    Body,
};

pub fn http_response_101(allocator: std.mem.Allocator) !HttpHeader {
    var header = HttpHeader{
        .fields = std.StringHashMap([]const u8).init(allocator),
        .status_code = 101,
    };
    try header.fields.put("Upgrade", "websocket");
    try header.fields.put("Connection", "Upgrade");
    try header.fields.put("Sec-WebSocket-Version", "13");
    return header;
}

pub fn parse_http_request(allocator: std.mem.Allocator, raw_data: []u8) !HttpHeader {
    var request = HttpHeader{
        .fields = std.StringHashMap([]const u8).init(allocator),
    };

    var header_name: ?[]u8 = undefined;
    var state: HttpParserState = .Method;
    var start: usize = 0;

    for (raw_data, 0..) |byte, end| {
        switch (state) {
            .Method => {
                if (byte == ' ') {
                    if (!eql(u8, raw_data[start..end], "GET")) {
                        return error.IllegalMethode;
                    }
                    start = end + 1;
                    state = .Resource;
                }
            },
            .Resource => {
                if (byte == ' ') {
                    request.resource = raw_data[start..end];
                    start = end + 1;
                    state = .Protocol;
                }
            },
            .Protocol => {
                if (byte == '\n') {
                    // end-1 because of the \r
                    const proto = raw_data[start .. end - 1];
                    if (!eql(u8, proto, "HTTP/1.1") and !eql(u8, proto, "HTTP/1.0")) {
                        return error.ProtocolNotSupported;
                    }
                    start = end + 1;
                    state = .HeaderName;
                }
            },
            .HeaderName => {
                switch (byte) {
                    '\n' => {
                        start = end + 1;
                        state = .Body;
                    },
                    ':' => {
                        header_name = raw_data[start..end];
                    },
                    ' ' => {
                        // only if the header name is already set
                        if (header_name) |_| {
                            start = end + 1;
                            state = .HeaderValue;
                        }
                    },
                    else => {},
                }
            },
            .HeaderValue => {
                if (byte == '\n') {
                    if (header_name) |name| {
                        // -1 because of the \r
                        const header_value = raw_data[start .. end - 1];
                        try request.fields.put(std.ascii.lowerString(name, name), header_value);
                    }
                    start = end + 1;
                    state = .HeaderName;
                }
            },
            .Body => {
                request.body = raw_data[start..];
                break;
            },
        }
    }

    if (state != .Body) {
        return error.IllegalHttpRequest;
    }

    return request;
}
