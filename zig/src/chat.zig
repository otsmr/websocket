//
// Copyright (c) 2024, Tobias Müller <git@tsmr.eu>
//

const std = @import("std");
const eql = std.mem.eql;
const Dataframe = @import("dataframe.zig").Dataframe;
const base64 = @import("base64.zig");
const ws = @import("websocket.zig");

pub const ChatContext = struct {
    users: std.StringHashMap(*User),
    sessions: std.StringHashMap([]const u8),
    allocator: std.mem.Allocator,

    // Add so each connection can check if it has a new connection
    onlineUsers: std.StringHashMap(std.AutoHashMap(usize, *ws.WebSocketConnection)),
    onlineUserChangedUsers: u64 = 0,

    pub fn init(allocator: std.mem.Allocator) !ChatContext {
        return ChatContext{ .users = std.StringHashMap(*User).init(allocator), .onlineUsers = std.StringHashMap(std.AutoHashMap(usize, *ws.WebSocketConnection)).init(allocator), .sessions = std.StringHashMap([]const u8).init(allocator), .allocator = allocator };
    }

    pub fn deinit(self: *ChatContext) void {
        // TODO: Release keys and values!!

        self.users.deinit();
        self.sessions.deinit();
    }
};

pub const User = struct {
    username: [:0]const u8,
    password: [:0]const u8,
};

const Message = struct { from: []const u8, to: []const u8, message: []const u8, created_at: u64 };

const Error = struct { message: []const u8 };

const OnlineUsers = struct { usernames: [][]const u8 };

const Item = struct { kind: []const u8, online_users: ?OnlineUsers = null, err: ?Error = null, user: ?User = null, message: ?Message = null, session_id: ?[]const u8 = null };

pub const ChatHandler = struct {
    conn: *ws.WebSocketConnection,
    context: *ChatContext,
    username: ?[]const u8 = null,
    user_connection_id: usize = undefined,
    onlineUserChangedUsers: u64 = 0,

    pub fn init(conn: *ws.WebSocketConnection, context: *ChatContext) !ChatHandler {
        return .{ .conn = conn, .context = context };
    }

    pub fn cronJob(self: *ChatHandler) !void {
        if (self.username == null) {
            return;
        }
        if (self.onlineUserChangedUsers == self.context.onlineUserChangedUsers) {
            // nothing has changed
            return;
        }
        self.onlineUserChangedUsers = self.context.onlineUserChangedUsers;
        var userList = try self.conn.allocator.alloc([]const u8, self.context.onlineUsers.count());
        defer self.conn.allocator.free(userList);
        var users = self.context.onlineUsers.keyIterator();
        var i: usize = 0;
        while (users.next()) |username| : (i += 1) {
            userList[i] = username.*;
        }
        var item = Item{ .kind = "update-online-users", .online_users = OnlineUsers{ .usernames = userList } };
        const resp = try std.json.stringifyAlloc(self.context.allocator, item, .{});
        defer self.conn.allocator.free(resp);
        try self.conn.sendText(resp);
    }

    pub fn onClose(self: *ChatHandler) !void {
        try self.removeFromOnlineUsers();
    }

    pub fn onError(self: *ChatHandler) !void {
        try self.removeFromOnlineUsers();
        // remove from context
    }

    pub fn onMessage(self: *ChatHandler, data: ws.WebSocketData) !void {
        std.log.info("New message: {s}", .{data.payload});

        const parsed = std.json.parseFromSlice(Item, self.context.allocator, data.payload, .{}) catch |err| {
            std.log.warn("Error: {any}", .{err});
            return;
        };
        defer parsed.deinit();
        const request = parsed.value;

        if (eql(u8, request.kind, "login")) {
            if (request.user) |user| {
                try self.handle_login(user);
            }
        } else if (eql(u8, request.kind, "check-session")) {
            if (request.session_id) |session_id| {
                try self.handle_check_session(session_id);
            }
        } else if (eql(u8, request.kind, "new-message")) {
            if (request.message) |message| {
                try self.handle_new_message(message);
            }
        }
    }

    pub fn removeFromOnlineUsers(self: *ChatHandler) !void {
        if (self.username == null) {
            return;
        }
        var connections = self.context.onlineUsers.get(self.username.?);
        if (connections != null) {
            _ = connections.?.remove(self.user_connection_id);

            // Why does connections.?.count() does not work?
            var connections_keys = connections.?.keyIterator();
            var open_connections: usize = 0;
            while (connections_keys.next()) |_| : (open_connections += 1) {}

            // User is not online any more so remove from online users
            if (open_connections == 0) {
                _ = self.context.onlineUsers.remove(self.username.?);
            }
        }
        // indicate that there was a change in the online users
        self.context.onlineUserChangedUsers += 1;
    }

    pub fn sendErrorMsg(self: *ChatHandler, msg: [:0]const u8) !void {
        const item = Item{ .kind = "error", .err = Error{ .message = msg } };
        const sessionResp = try std.json.stringifyAlloc(self.context.allocator, item, .{});
        defer self.context.allocator.free(sessionResp);
        try self.conn.sendText(sessionResp);
    }

    pub fn addToOnlineUsers(self: *ChatHandler) !void {
        if (self.username == null) {
            return;
        }
        var userConnections = self.context.onlineUsers.get(self.username.?);

        if (userConnections != null) {
            // User can have up to 1000 connections
            for (0..1000) |i| {
                if (userConnections.?.getKey(i) == null) {
                    self.user_connection_id = i;
                    try userConnections.?.put(self.user_connection_id, self.conn);
                    break;
                }
            }
        } else {
            self.user_connection_id = 0;
            var connections = std.AutoHashMap(usize, *ws.WebSocketConnection).init(self.context.allocator);
            try connections.put(self.user_connection_id, self.conn);
            try self.context.onlineUsers.put(self.username.?, connections);
        }
        // indicate that there was a change in the online users
        self.context.onlineUserChangedUsers += 1;
    }

    pub fn handle_new_message(self: *ChatHandler, message: Message) !void {
        if (self.username == null) {
            return;
        }
        if (!eql(u8, message.from, self.username.?)) {
            try self.sendErrorMsg("Hello? You can't just pretend to be someone else!1!");
            return;
        }

        const otherHandlers = self.context.onlineUsers.get(message.to);
        if (otherHandlers == null) {
            try self.sendErrorMsg("User is not online. Please try again later!");
            return;
        }

        var item = Item{
            .kind = "new-message",
            .message = message,
        };
        const sendMessage = try std.json.stringifyAlloc(self.context.allocator, item, .{});
        defer self.context.allocator.free(sendMessage);

        var handlers = otherHandlers.?.valueIterator();
        while (handlers.next()) |handler| {
            // std.log.info("Found handler: {d}", .{handler.*});
            try handler.*.sendText(sendMessage);
        }

        const myConnections = self.context.onlineUsers.get(message.from);
        var myHandlers = myConnections.?.valueIterator();
        while (myHandlers.next()) |handler| {
            try handler.*.sendText(sendMessage);
        }
    }

    pub fn handle_login(self: *ChatHandler, user: User) !void {
        var pwhashBuffer = try self.context.allocator.alloc(u8, 20);
        var pwhash = pwhashBuffer[0..20];

        std.crypto.hash.Sha1.hash(user.password, pwhash, .{});

        var heapUsername = try self.context.allocator.alloc(u8, user.username.len);
        std.mem.copy(u8, heapUsername, user.username);

        var db_user = self.context.users.get(user.username);
        if (db_user) |dbuser| {
            // User exists check Credentials
            if (!std.mem.eql(u8, pwhash, dbuser.password)) {
                try self.sendErrorMsg("Password is wrong!");
                return;
            }
        } else {
            if (heapUsername.len > 20) {
                try self.sendErrorMsg("Username to long! Max 20 chars");
                return;
            }
            for (heapUsername) |char| {
                if (!std.ascii.isAlphanumeric(char)) {
                    try self.sendErrorMsg("Only Alphanumeric chars allowed (A-Z, a-z, 0-9)!");
                    return;
                }
            }

            var heapUser = try self.context.allocator.create(User);
            heapUser.username = @ptrCast(heapUsername);
            heapUser.password = @ptrCast(pwhashBuffer);
            try self.context.users.put(heapUsername, heapUser);
        }

        const rand = std.crypto.random;
        var session_id: [20]u8 = undefined;
        rand.bytes(&session_id);

        var session_id_b64_buffer: [30]u8 = undefined;
        var session_id_b64 = try base64.encode(&session_id, &session_id_b64_buffer);

        var heapSessionId = try self.context.allocator.alloc(u8, session_id_b64.len);
        std.mem.copy(u8, heapSessionId, session_id_b64);

        try self.context.sessions.put(heapSessionId, @ptrCast(heapUsername));

        // remove password
        const respUser = User{ .username = user.username, .password = "[redacted]" };

        const item = Item{ .kind = "session", .session_id = session_id_b64, .user = respUser };
        const sessionResp = try std.json.stringifyAlloc(self.context.allocator, item, .{});
        defer self.context.allocator.free(sessionResp);

        self.username = @ptrCast(heapUsername);
        try self.addToOnlineUsers();
        try self.conn.sendText(sessionResp);
    }

    pub fn handle_check_session(self: *ChatHandler, session_id: []const u8) !void {
        var db_username = self.context.sessions.get(session_id);
        var item: ?Item = null;
        var respUser: User = undefined;

        if (db_username) |username| {
            respUser = User{ .username = @ptrCast(username), .password = "[redacted]" };
            item = Item{ .kind = "session", .session_id = session_id, .user = respUser };

            var heapUsername = try self.context.allocator.alloc(u8, username.len);
            std.mem.copy(u8, heapUsername, username);

            self.username = @ptrCast(heapUsername);
            try self.addToOnlineUsers();
        }

        if (item == null) {
            item = Item{ .kind = "invalid-session" };
        }

        const sessionResp = try std.json.stringifyAlloc(self.context.allocator, item.?, .{});
        defer self.context.allocator.free(sessionResp);

        try self.conn.sendText(sessionResp);
    }
};
