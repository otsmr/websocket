const std = @import("std");
const base64 = @import("base64.zig");
const Dataframe = @import("dataframe.zig").Dataframe;
const http = @import("http.zig");

const net = std.net;
const os = std.os;
const Allocator = std.mem.Allocator;

pub const Config = struct { port: u16 = 3000, address: []const u8 = "127.0.0.1", handshake_timeout_ms: u32 = 10000 };

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

pub const WebSocketServer = struct {
    config: Config,
    allocator: Allocator,

    pub fn init(allocator: Allocator, config: Config) !WebSocketServer {
        return .{ .config = config, .allocator = allocator };
    }

    fn accept(self: *WebSocketServer, comptime H: type, context: anytype, stream: net.Stream) !void {
        errdefer stream.close();

        std.log.info("New connection received!", .{});

        var conn = WebSocketConnection{ .stream = stream, .allocator = self.allocator };
        var handler = try H.init(&conn, context);

        conn.do_handshake(self.config.handshake_timeout_ms) catch |err| {
            try stream.writeAll("HTTP/1.1 400 Bad Request\r\n\r\n");
            return err;
        };

        conn.state = .WaitingForFrames;

        std.log.info("New connection established!", .{});

        try conn.read_loop(H, &handler);
    }
};

const ConnectionState = enum(usize) {
    Disconnected = 0,
    CloseFromServer,
    WaitingForFrames = 10,
    InDataPayload, // with socket.read each time only 1024 bytes
};

pub const WebSocketConnection = struct {
    stream: net.Stream,
    allocator: Allocator,
    state: ConnectionState = .Disconnected,

    fn read_loop(self: *WebSocketConnection, comptime H: type, handler: *H) !void {
        var read_buffer: [1024]u8 = undefined;
        var frame_not_fully_received: ?Dataframe = null;
        var continuation_frames: []Dataframe = try self.allocator.alloc(Dataframe, 100);
        defer self.allocator.free(continuation_frames);
        var continuation_frames_len: usize = 0;

        while (true) {
            const size = try self.stream.read(&read_buffer);

            if (size == 0) {
                self.state = .Disconnected;
                break;
            }
            var offset: usize = 0;

            while (offset < size) {
                const buf = read_buffer[offset..size];

                var frame: Dataframe = undefined;

                if (frame_not_fully_received != null) {
                    var missing = frame_not_fully_received.?.get_missing_payload_size();
                    if (missing > buf.len) {
                        missing = buf.len;
                    }
                    try frame_not_fully_received.?.add_payload(buf[0..missing]);
                    offset += missing;
                    frame = frame_not_fully_received.?;
                    frame_not_fully_received = null;
                } else {
                    frame = try Dataframe.parse(self.allocator, buf);
                    offset += frame.consumed_len;
                }

                if (frame.is_fully_received()) {
                    if (frame.flags.fin) {
                        std.log.warn("Handler should get only ONE CUSTOM Data object");
                        switch (frame.opcode) {
                            .ContinuationFrame => {
                                std.log.info("Got {d} ContinuationFrames", .{continuation_frames_len});
                                // get collected frames
                                continuation_frames_len = 0;
                            },
                            else => {
                                // send to the handle only a DATA object
                                // containing only the type and the len
                                try handler.handle(frame);
                            },
                        }
                    } else {
                        if (continuation_frames_len > continuation_frames.len) {
                            continuation_frames = try self.allocator.realloc(continuation_frames, continuation_frames_len + 10);
                        }
                        continuation_frames[continuation_frames_len] = frame;
                        continuation_frames_len += 1;
                    }
                } else {
                    frame_not_fully_received = frame;
                }
            }

            // if (comptime std.meta.hasFn(H, "afterInit")) {
            // 	handler.afterInit() catch return;
            // }
        }
        self.stream.close();
    }

    fn do_handshake(self: *WebSocketConnection, timeout: ?u32) !void {
        var buffer: [1024]u8 = undefined;

        var raw_request = try self.receive_handshake(&buffer, timeout);

        var http_request = try http.parse_http_request(self.allocator, raw_request);
        defer http_request.fields.deinit();
        errdefer http_request.fields.deinit();

        var websocket_key = http_request.fields.get("sec-websocket-key");
        if (websocket_key) |wke| {
            if (wke.len != 24) {
                return error.InvalidSecWebSocketKey;
            }

            const hardcoded_key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"[0..36];
            var request_key: [24 + 36]u8 = undefined;
            std.mem.copy(u8, request_key[0..], wke);
            std.mem.copy(u8, request_key[24..], hardcoded_key);

            var hash: [20]u8 = undefined;

            std.crypto.hash.Sha1.hash(&request_key, &hash, .{});

            var response_key_buffer: [30]u8 = undefined;
            var response_key = try base64.encode(&hash, &response_key_buffer);

            var http_response = try http.http_response_101(self.allocator);
            try http_response.fields.put("Sec-WebSocket-Accept", response_key);

            var response_buffer: [250]u8 = undefined;
            const response = try http_response.to_str(&response_buffer);
            try self.stream.writeAll(response);
        } else {
            return error.SecWebSocketKeyMissing;
        }
    }

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
                    const read_no_timeout = std.mem.toBytes(os.timeval{
                        .tv_sec = 0,
                        .tv_usec = 0,
                    });
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
};
