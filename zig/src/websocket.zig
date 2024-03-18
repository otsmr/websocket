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
            if (std.Thread.spawn(.{}, WebSocketServer.accept_without_error, args)) |handle| {
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

    fn accept_without_error(self: *WebSocketServer, comptime H: type, context: anytype, stream: net.Stream) void {
        self.accept(H, context, stream) catch |err| {
            std.log.warn("Error in connection: {any}", .{err});
        };
    }

    fn accept(self: *WebSocketServer, comptime H: type, context: anytype, stream: net.Stream) !void {
        errdefer stream.close();

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
                try handler.onError();
            }
            return err;
        };

        if (@hasDecl(H, "onClose")) {
            try handler.onClose();
        }

        stream.close();
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
        var buffer: []u8 = try self.allocator.alloc(u8, text.len);
        std.mem.copy(u8, buffer, text);
        const data = WebSocketData{ .type = .Text, .payload = buffer[0..text.len] };
        try self.send(data);
    }

    // Takes ownership of data
    pub fn send(self: *WebSocketConnection, data: WebSocketData) !void {
        var frame = try Dataframe.from_websocket_data(self.allocator, data);
        defer frame.deinit();
        var raw_bytes = try frame.to_raw_bytes();
        self.stream.writeAll(raw_bytes) catch |err| {
            std.log.warn("Error sending!", .{});
            // TODO: handle error and close socket
            return err;
        };
    }

    fn read_loop(self: *WebSocketConnection, comptime H: type, handler: *H) !void {
        var read_buffer: [1024]u8 = undefined;
        var is_frame_not_fully_received: bool = false;
        var read_buffer_offset: usize = 0;

        var frame: Dataframe = undefined;

        var continuation_frames: []Dataframe = try self.allocator.alloc(Dataframe, 10);
        defer self.allocator.free(continuation_frames);

        var continuation_frames_len: usize = 0;

        const cronJobTimeout = 1000;
        const cronjob_timeout = std.mem.toBytes(os.timeval{
            .tv_sec = @intCast(@divTrunc(cronJobTimeout, 1000)),
            .tv_usec = @intCast(@mod(cronJobTimeout, 1000) * 1000),
        });

        var send_ping_timeout: ?i64 = null;
        var waiting_for_pong: bool = false;

        try std.os.setsockopt(self.stream.handle, os.SOL.SOCKET, os.SO.RCVTIMEO, &cronjob_timeout);

        read_from_stream: while (true) {
            var size = self.stream.read(read_buffer[read_buffer_offset..]) catch |err| {
                if (err == error.WouldBlock) {
                    if (@hasDecl(H, "cronJob")) {
                        handler.cronJob() catch return;
                    }

                    // check if waiting for pong
                    if (waiting_for_pong) {
                        return error.DidNotRecvPong;
                    }

                    // check if it is time to send a ping
                    if (send_ping_timeout) |first_timeout| {
                        var current_timeout = std.time.milliTimestamp() - first_timeout;

                        if (current_timeout > (1000 * 60 * 2)) {
                            // after two minutes of inactivity send ping request
                            waiting_for_pong = true;
                            var ping = try Dataframe.get_ping(self.allocator);
                            try self.stream.writeAll(try ping.to_raw_bytes());
                        }
                    } else {
                        send_ping_timeout = std.time.milliTimestamp();
                    }

                    continue;
                }

                return err;
            };
            send_ping_timeout = null;
            size += read_buffer_offset;
            read_buffer_offset = 0;

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

                if (is_frame_not_fully_received) {
                    var missing = frame.get_missing_payload_size();
                    if (missing > buf.len) {
                        missing = buf.len;
                    }
                    try frame.add_payload(buf[0..missing]);
                    offset += missing;
                    is_frame_not_fully_received = false;
                } else {
                    frame = Dataframe.parse(self.allocator, buf) catch |err| {
                        if (err == error.HeaderToShort or err == error.NoHeaderFound) {
                            read_buffer_offset = size;
                            continue :read_from_stream;
                        }
                        return err;
                    };
                    offset += frame.consumed_len;
                }

                std.log.info("Got Len {d} {d} {d} {d} filled {d}", .{ offset, size, frame.consumed_len, frame.payload.len, frame.payload_filled_len });

                if (!frame.is_fully_received()) {
                    is_frame_not_fully_received = true;
                    continue;
                }

                if (!frame.flags.fin) {
                    if (continuation_frames_len == 0 and frame.opcode == .ContinuationFrame) {
                        return error.NoMessageToContinue;
                    }
                    if (continuation_frames_len > 0 and frame.opcode != .ContinuationFrame) {
                        return error.ContinuationOpcodeMustBeSet;
                    }
                    if ((continuation_frames_len + 3) > continuation_frames.len) {
                        continuation_frames = try self.allocator.realloc(continuation_frames, continuation_frames_len + 10);
                    }
                    continuation_frames[continuation_frames_len] = frame;
                    // make dummy frame
                    frame = try Dataframe.get_ping(self.allocator);
                    continuation_frames_len += 1;
                    continue;
                }

                if (frame.flags.fin and continuation_frames_len > 0) {
                    if (frame.opcode == .TextFrame or frame.opcode == .BinaryFrame) {
                        return error.ContinuationOpcodeMustBeSetForNonControlFrames;
                    }
                }

                switch (frame.opcode) {
                    .Ping => {
                        if (frame.payload.len > 125) {
                            // control frames MUST have a payload length of 125 bytes or less
                            var closing_frame = try Dataframe.closing(self.allocator, 1002);
                            defer closing_frame.deinit();
                            try self.stream.writeAll(try closing_frame.to_raw_bytes());
                            return;
                        }
                        var pong = try Dataframe.get_pong(self.allocator, frame.payload);
                        defer pong.deinit();
                        try self.stream.writeAll(try pong.to_raw_bytes());
                    },
                    .Pong => {
                        waiting_for_pong = false;
                        frame.deinit();
                    },
                    .ConectionClose => {
                        var code: u16 = frame.get_closing_code() catch 1002;

                        // invalid closing codes
                        if (code < 1000 or code == 1004 or code == 1005 or code == 1006 or (code > 1013 and code < 3000)) {
                            code = 1002;
                        }

                        if (frame.payload.len > 125) {
                            // control frames MUST have a payload length of 125 bytes or less
                            code = 1002;
                        }

                        if (frame.payload.len > 2) {
                            if (!std.unicode.utf8ValidateSlice(frame.payload[2..])) {
                                // must be valid utf8
                                code = 1002;
                            }
                        }

                        var closing_frame = try Dataframe.closing(self.allocator, code);
                        defer closing_frame.deinit();

                        try self.stream.writeAll(try closing_frame.to_raw_bytes());
                        return;
                    },
                    .ContinuationFrame => {
                        continuation_frames[continuation_frames_len] = frame;
                        continuation_frames_len += 1;

                        var data_type: WebSocketDataType = .Text;
                        if (continuation_frames[0].opcode == .BinaryFrame) {
                            data_type = .Binary;
                        } else if (continuation_frames[0].opcode != .TextFrame) {
                            return error.OpcodeDoesNotSupportContinuation;
                        }

                        var payload_size_full: u64 = 0;
                        for (0..continuation_frames_len) |i| {
                            payload_size_full += continuation_frames[i].payload.len;
                        }

                        var data_buf = try self.allocator.alloc(u8, payload_size_full);
                        // defer self.allocator.free(data_buf);

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
                        // takes ownership if the dataframes buffer
                        const data = WebSocketData{ .type = data_type, .payload = frame.payload };
                        try handler.onMessage(data);
                    },
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
            defer http_response.deinit();

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
