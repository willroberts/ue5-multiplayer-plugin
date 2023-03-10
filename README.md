# Multiplayer Session Plugin for Unreal Engine

This plugin provides an implementation of the Online Subsystem using the Steam provider.

When using the plugin, players can host and join games when logged into Steam. The hosting player will act as a listen server, resulting in a peer-to-peer network topology.

The plugin also provides a debug menu, and a corresponding Widget for the debug menu.

Dedicated server functionality is planned. Support for Epic Online Services (as an alternative to Steam) is also planned.

Developed with Unreal Engine 5 and C++.

## Polish Burndown List

- Finish all delegates and handlers
	- Destroy?
	- Start vs Create?
- Better code organization
- Better comments and documentation
- Expose all assumptions as configurables (Steam, result cap, etc.)
- Improve all error handling
	- Utility function to log to screen (GEngine) and text (ULOG)
	- Parameter to mark things as errors (GEngine red color, ULOG 'ERROR:' prefix)
- Polish and decouple debug menu where possible
- Port plugin to a new project/repo without added UE content
- Support more than one session (NAME_GameSession)