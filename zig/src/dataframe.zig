const std = @import("std");

const DataframeType = enum {
    Text,
    Bin,
};

pub const Dataframe = struct {
    type: DataframeType,
    data: [1024]u8 = undefined,

    pub fn parse(raw: []const u8) !Dataframe {
        var df = Dataframe{ .type = .Text };

        std.mem.copy(u8, &df.data, raw);

        return df;
    }
};
