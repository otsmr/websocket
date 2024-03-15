//
// Copyright (c) 2024, Tobias Müller <git@tsmr.eu>
//

const std = @import("std");
const Dataframe = @import("dataframe.zig").Dataframe;
const base64 = @import("base64.zig");
const ws = @import("websocket.zig");

pub const ChatContext = struct { users: std.StringHashMap(*User), allocator: std.mem.Allocator };

pub const User = struct {
    username: [:0]const u8,
    password: [:0]const u8,
};

const Message = struct { from: []const u8, to: []const u8, message: []const u8, created_at: u64 };

const Error = struct { message: []const u8 };

const Item = struct { kind: []const u8, err: ?Error = null, user: ?User = null, message: ?Message = null, session_id: ?[]const u8 = null };

pub const ChatHandler = struct {
    conn: *ws.WebSocketConnection,
    context: *ChatContext,

    pub fn init(conn: *ws.WebSocketConnection, context: *ChatContext) !ChatHandler {
        return .{ .conn = conn, .context = context };
    }

    pub fn handle_login(self: *ChatHandler, user: User) !void {
        var pwhashBuffer = try self.context.allocator.alloc(u8, 20);
        var pwhash = pwhashBuffer[0..20];

        std.crypto.hash.Sha1.hash(user.password, pwhash, .{});

        var db_user = self.context.users.get(user.username);
        if (db_user) |dbuser| {
            // User exists check Credentials
            if (!std.mem.eql(u8, pwhash, dbuser.password)) {
                const item = Item{ .kind = "error", .err = Error{ .message = "Password is wrong!" } };
                const sessionResp = try std.json.stringifyAlloc(self.context.allocator, item, .{});
                try self.conn.sendText(sessionResp);
                return;
            }
        } else {
            var heapUser = try self.context.allocator.create(User);

            var username = try self.context.allocator.alloc(u8, user.username.len);
            std.mem.copy(u8, username, user.username);

            heapUser.username = @ptrCast(username);
            heapUser.password = @ptrCast(pwhashBuffer);

            try self.context.users.put(username, heapUser);
        }

        std.log.info("creating session", .{});

        const rand = std.crypto.random;
        var session_id: [20]u8 = undefined;
        rand.bytes(&session_id);

        var session_id_b64_buffer: [30]u8 = undefined;
        var session_id_b64 = try base64.encode(&session_id, &session_id_b64_buffer);

        // TODO: Remove password
        const item = Item{ .kind = "session", .session_id = session_id_b64, .user = user };
        const sessionResp = try std.json.stringifyAlloc(self.context.allocator, item, .{});
        defer self.context.allocator.free(sessionResp);

        std.log.info("Sending: {s}", .{sessionResp});
        try self.conn.sendText(sessionResp);
    }

    pub fn handle(self: *ChatHandler, data: ws.WebSocketData) !void {
        std.log.info("New message: {s}", .{data.payload});

        const parsed = std.json.parseFromSlice(Item, self.context.allocator, data.payload, .{}) catch |err| {
            std.log.warn("Error: {any}", .{err});
            return;
        };
        defer parsed.deinit();
        const request = parsed.value;

        if (std.mem.eql(u8, request.kind, "login")) {
            if (request.user) |user| {
                try self.handle_login(user);
            }
        }
    }
};
