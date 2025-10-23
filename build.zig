const std = @import("std");

pub fn build(b: *std.Build) void {
    const mod = b.createModule(.{
        .target = b.standardTargetOptions(.{}),
        .optimize = b.standardOptimizeOption(.{}),
        .link_libc = true,
    });

    mod.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{"main.c"},
        .language = .c,
        .flags = &.{"-std=c23"},
    });

    mod.linkSystemLibrary("opengl32", .{});
    mod.linkSystemLibrary("gdi32", .{});
    mod.linkSystemLibrary("winmm", .{});

    mod.addIncludePath(b.path("raylib/include"));
    mod.addObjectFile(b.path("raylib/lib/libraylib.a"));

    const exe = b.addExecutable(.{
        .name = "snake",
        .root_module = mod,
    });

    b.installArtifact(exe);
}
