//
// Copyright (c) 2024, Tobias Müller <git@tsmr.eu>
//

const std = @import("std");
const Dataframe = @import("dataframe.zig").Dataframe;
const ws = @import("websocket.zig");
const chat = @import("chat.zig");

const Context = struct {};

pub fn main() !void {
    var general_purpose_allocator = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = general_purpose_allocator.detectLeaks();

    const allocator = general_purpose_allocator.allocator();

    std.log.info("Starting WebSocket Server", .{});

    if (true) {
        var ctx = Context{};
        try ws.listen(AutoBahnHandler, allocator, &ctx, .{});
    } else {
        var ctx = try chat.ChatContext.init(allocator);
        defer ctx.deinit();

        while (true) {
            ws.listen(chat.ChatHandler, allocator, &ctx, .{}) catch |err| {
                if (err != error.AddressInUse) {
                    return err;
                }
            };
        }
    }
}

const AutoBahnHandler = struct {
    conn: *ws.WebSocketConnection,
    context: *Context,

    pub fn init(conn: *ws.WebSocketConnection, context: *Context) !AutoBahnHandler {
        return .{ .conn = conn, .context = context };
    }

    // Optional: Will be called every 5 seconds
    pub fn cronJob(self: *AutoBahnHandler) !void {
        _ = self;
    }

    // Optional: Will be called when the connection was closed
    pub fn onClose(self: *AutoBahnHandler) !void {
        _ = self;
    }

    // Optional: Will be called in case of an error and the connection was therefore closed
    pub fn onError(self: *AutoBahnHandler) !void {
        _ = self;
        // remove from context
    }

    // Required: Will be called when a new message was send
    pub fn onMessage(self: *AutoBahnHandler, data: ws.WebSocketData) !void {
        try self.conn.send(data);
    }
};

// Running all tests with zig build test, is there a better solution?
comptime {
    _ = @import("base64.zig");
}
