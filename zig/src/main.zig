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

const Handler = struct {
    conn: *ws.WebSocketConnection,
    context: *Context,

    pub fn init(conn: *ws.WebSocketConnection, context: *Context) !Handler {
        return .{ .conn = conn, .context = context };
    }

    // Optional: Will be called every 5 seconds
    pub fn cronJob(self: *Handler) !void {
        _ = self;
    }

    // Optional: Will be called when the connection was closed
    pub fn onClose(self: *Handler) !void {
        _ = self;
    }

    // Optional: Will be called in case of an error and the connection was therefore closed
    pub fn onError(self: *Handler) !void {
        _ = self;
        // remove from context
    }

    // Required: Will be called when a new message was send
    pub fn onMessage(self: *Handler, data: ws.WebSocketData) !void {
        _ = self;
        var pbuf = data.payload;
        // if (pbuf.len > 15) {
        //     pbuf = data.payload[0..15];
        // }

        std.log.info("New message: {s}", .{pbuf});

        // const resp = ws.WebSocketData{ .type = .Text, .payload = pbuf };

        // try self.conn.send(resp);
    }
};

// Running all tests with zig build test, is there a better solution?
comptime {
    _ = @import("base64.zig");
}
