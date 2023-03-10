# Multiplayer Session Plugin for Unreal Engine

This plugin provides an implementation of the Online Subsystem using the Steam provider.

When using the plugin, players can host and join games when logged into Steam. The hosting player will act as a listen server, resulting in a peer-to-peer network topology.

The plugin also provides a debug menu, and a corresponding Widget for the debug menu.

Dedicated server functionality is planned. Support for Epic Online Services (as an alternative to Steam) is also planned.

Developed with Unreal Engine 5.1 and C++.

## Usage

After adding the plugin to your project, use the `WBP_DebugMenu` asset to test functionality.

The `DebugMenu` C++ class serves as a reference implementation of the plugin.
