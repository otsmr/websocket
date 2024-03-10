const std = @import("std");
const base64 = @import("base64.zig");
const Dataframe = @import("dataframe.zig").Dataframe;
const parse_http_header = @import("http.zig").parse_http_header;

const net = std.net;
const os = std.os;
const Allocator = std.mem.Allocator;

const read_no_timeout = std.mem.toBytes(os.timeval{
    .tv_sec = 0,
    .tv_usec = 0,
});

pub const WebSocketConnection = struct {
    stream: net.Stream,
    allocator: Allocator,

    fn receive_handshake(self: *WebSocketConnection, buffer: []u8, timeout: ?u32) ![]u8 {
        var deadout: ?i64 = null;
        if (timeout) |ms| {
            // our timeout for each individual read
            var read_timeout: ?[@sizeOf(os.timeval)]u8 = null;
            read_timeout = std.mem.toBytes(os.timeval{
                .tv_sec = @intCast(@divTrunc(ms, 1000)),
                .tv_usec = @intCast(@mod(ms, 1000) * 1000),
            });

            if (read_timeout) |to| {
                try std.os.setsockopt(self.stream.handle, os.SOL.SOCKET, os.SO.RCVTIMEO, &to);
            }

            deadout = std.time.milliTimestamp() + ms;
        }

        var total: usize = 0;
        while (true) {
            const size = try self.stream.read(buffer[total..]);

            if (size == 0) {
                return error.Invalid;
            }

            total += size;

            const request = buffer[0..total];

            if (std.mem.endsWith(u8, request, "\r\n\r\n") or std.mem.endsWith(u8, request, "\n\n")) {
                if (timeout != null) {
                    try os.setsockopt(self.stream.handle, os.SOL.SOCKET, os.SO.RCVTIMEO, &read_no_timeout);
                }
                return request;
            }

            if (deadout) |dl| {
                if (std.time.milliTimestamp() > dl) {
                    return error.Timeout;
                }
            }
        }
    }
    pub fn do_handshake(self: *WebSocketConnection, timeout: ?u32) !void {
        var buffer: [1024]u8 = undefined;

        var raw_request = try self.receive_handshake(&buffer, timeout);

        var http_request = try parse_http_header(self.allocator, raw_request);

        std.log.info("Complete HS receive: {!}", .{http_request});
    }

    pub fn read_loop(self: *WebSocketConnection, comptime H: type, handler: *H) !void {
        var buffer: [1024]u8 = undefined;

        while (true) {
            const size = try self.stream.read(&buffer);
            if (size == 0)
                break;

            const bf = buffer[0..size];
            var df = try Dataframe.parse(bf);

            try handler.handle(df);

            // if (comptime std.meta.hasFn(H, "afterInit")) {
            // 	handler.afterInit() catch return;
            // }
        }
    }
};

pub const WebSocketServer = struct {
    config: Config,
    allocator: Allocator,

    pub fn init(allocator: Allocator, config: Config) !WebSocketServer {
        return .{ .config = config, .allocator = allocator };
    }

    fn accept(self: *WebSocketServer, comptime H: type, context: anytype, stream: net.Stream) !void {
        errdefer stream.close();

        std.log.info("Connection received!", .{});

        var conn = WebSocketConnection{ .stream = stream, .allocator = self.allocator };
        var handler = try H.init(&conn, context);
        // do handshake
        try conn.do_handshake(self.config.handshake_timeout_ms);
        try conn.read_loop(H, &handler);
    }
};

pub const Config = struct { port: u16 = 8080, address: []const u8 = "127.0.0.1", handshake_timeout_ms: u32 = 10000 };

pub fn listen(comptime H: type, allocator: Allocator, context: anytype, config: Config) !void {
    var wsserver = try WebSocketServer.init(allocator, config);

    const loopback = try net.Ip4Address.parse(config.address, config.port);
    const localhost = net.Address{ .in = loopback };

    var server = net.StreamServer.init(net.StreamServer.Options{});
    defer server.deinit();

    try server.listen(localhost);

    std.log.info("Listening on {}, access this port to end the program\n", .{server.listen_address.getPort()});

    while (true) {
        if (server.accept()) |conn| {
            const args = .{ &wsserver, H, context, conn.stream };
            if (std.Thread.spawn(.{}, WebSocketServer.accept, args)) |handle| {
                handle.detach();
            } else |err| {
                std.log.err("Failed to accept connection {}", .{err});
            }
        } else |err| {
            std.log.err("Failed to accept connection {}", .{err});
        }
    }
}
