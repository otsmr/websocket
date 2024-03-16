//
// Copyright (c) 2024, Tobias Müller <git@tsmr.eu>
//

const std = @import("std");
const base64 = @import("base64.zig");
const Dataframe = @import("dataframe.zig").Dataframe;
const http = @import("http.zig");

const net = std.net;
const os = std.os;
const Allocator = std.mem.Allocator;

pub const WebSocketDataType = enum { Text, Binary };

pub const WebSocketData = struct {
    type: WebSocketDataType,
    payload: []u8,
};

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

        conn.read_loop(H, &handler) catch |err| {
            std.log.warn("Connection error {any}", .{err});

            if (@hasDecl(H, "onError")) {
                // if (comptime std.meta.hasFn(H, "onError")) {
                try handler.onError();
                return;
            }
        };

        std.log.info("Connection is closing", .{});

        if (@hasDecl(H, "onClose")) {
            std.log.info("Handler was called", .{});
            try handler.onClose();
            return;
        }

        self.stream.close();
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

    pub fn sendText(self: *WebSocketConnection, text: []const u8) !void {
        var buffer: [1000]u8 = undefined;
        std.mem.copy(u8, &buffer, text);
        const data = WebSocketData{ .type = .Text, .payload = buffer[0..text.len] };
        try self.send(data);
    }

    pub fn send(self: *WebSocketConnection, data: WebSocketData) !void {
        var frame = try Dataframe.from_websocket_data(self.allocator, data);
        var raw_bytes = try frame.to_raw_bytes();

        std.log.info("Sending raw bytes", .{});
        self.stream.writeAll(raw_bytes) catch |err| {
            std.log.warn("Error sending!", .{});
            // TODO: handle error and close socket
            return err;
        };
    }

    fn read_loop(self: *WebSocketConnection, comptime H: type, handler: *H) !void {
        var read_buffer: [1024]u8 = undefined;
        var frame_not_fully_received: ?Dataframe = null;
        defer if (frame_not_fully_received != null) frame_not_fully_received.?.deinit();

        var continuation_frames: []Dataframe = try self.allocator.alloc(Dataframe, 10);
        defer self.allocator.free(continuation_frames);

        var continuation_frames_len: usize = 0;

        const cronJobTimeout = 1000;
        const cronjob_timeout = std.mem.toBytes(os.timeval{
            .tv_sec = @intCast(@divTrunc(cronJobTimeout, 1000)),
            .tv_usec = @intCast(@mod(cronJobTimeout, 1000) * 1000),
        });

        try std.os.setsockopt(self.stream.handle, os.SOL.SOCKET, os.SO.RCVTIMEO, &cronjob_timeout);

        while (true) {
            const size = self.stream.read(&read_buffer) catch |err| {
                if (err == error.WouldBlock) {
                    if (@hasDecl(H, "cronJob")) {
                        handler.cronJob() catch return;
                    }
                    continue;
                }

                return err;
            };

            if (size == 0) {
                self.state = .Disconnected;
                break;
            }

            if (@hasDecl(H, "cronJob")) {
                handler.cronJob() catch return;
            }

            var offset: usize = 0;

            while (offset < size) {
                const buf = read_buffer[offset..size];

                var frame: Dataframe = undefined;
                defer frame.deinit();

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
                        switch (frame.opcode) {
                            .Ping => {
                                unreachable;
                            },
                            .Pong => {
                                unreachable;
                            },
                            .ConectionClose => {
                                std.log.warn("Implement ConnectionClose", .{});
                                return;
                            },
                            .ContinuationFrame => {
                                continuation_frames[continuation_frames_len] = frame;
                                continuation_frames_len += 1;

                                var data_type: WebSocketDataType = .Text;
                                if (continuation_frames[0].opcode == .BinaryFrame) {
                                    data_type = .Binary;
                                } else {
                                    return error.OpcodeDoesNotSupportContinuation;
                                }

                                var payload_size_full: u64 = 0;
                                for (0..continuation_frames_len) |i| {
                                    payload_size_full += continuation_frames[i].payload.len;
                                }

                                var data_buf = try self.allocator.alloc(u8, payload_size_full);
                                var cursor: u64 = 0;
                                for (0..continuation_frames_len) |i| {
                                    // payload_size_full += continuation_frames[i].payload.len;
                                    std.mem.copy(u8, data_buf[cursor..], continuation_frames[i].payload);
                                    cursor += continuation_frames[i].payload.len;
                                    // free allocated memory
                                    continuation_frames[i].deinit();
                                }
                                // get collected frames
                                continuation_frames_len = 0;
                                const data = WebSocketData{ .type = data_type, .payload = data_buf };
                                try handler.onMessage(data);
                            },
                            else => {
                                var data_type: WebSocketDataType = .Text;
                                if (frame.opcode == .BinaryFrame) {
                                    data_type = .Binary;
                                }
                                const data = WebSocketData{ .type = data_type, .payload = frame.payload };
                                try handler.onMessage(data);
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
        }
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
