# Multiplayer Session Plugin for Unreal Engine

This plugin provides an implementation of Unreal Engine's Online Subsystem, supporting multiple providers such as Steam, Epic Online Services, and LAN games.

When using the plugin, players can host and join multiplayer games over the Internet. The hosting player will act as a listen server, resulting in a peer-to-peer network topology.

The plugin also provides a debug menu, and a corresponding Widget for the debug menu.

Developed with Unreal Engine 5 and modern C++.

## Functionality

When playing in editor, or with development/debug builds, the OnlineSubsystem will be `NULL`, resulting in using LAN for connections.

When packaging or launching the project, the project's default OnlineSubsystem will be used. The Steam OnlineSubsystem is recommended.

The plugin can be used to create, start, find, join, or destroy multiplayer sessions.

## Installation

To install the plugin, simply copy the contents of this repository to the `Plugins/` folder in your project. You can clone the repo, or download an archive from the Releases page.

After reopening the project, you will be prompted to rebuild the plugin.

## Configuration (Steam)

Before using the plugin, there are some changes which must be made in the project:

1. Enable the `Online Subsystem Steam` plugin.
2. Update `Config/DefaultEngine.ini` according to [the `Online Subsystem Steam` docs](https://docs.unrealengine.com/5.1/en-US/ProgrammingAndScripting/Online/Steam/).
3. Update `Config/DefaultGame.ini` to add the following:

```ini
[/Script/Engine.GameSession]
MaxPlayers=64
```

## Configuration (Epic Online Services)

1. Enable the `Online Subsystem EOS` and `EOS Shared` plugins.
2. Configure the `Online Subsystem EOS` plugin as desired by setting the following options:
  - Artifact
    - Create a new artifact, and set its name and IDs
    - Set the default artifact name
  - EOS Plus Login Settings
    - Use Epic Account for EOS login
    - Use Crossplatform User IDs for EOS login
  - Crossplay Settings
    - Mirror achievements to EOS
    - Mirror presence to EAS
    - Mirror stats to EOS
    - Use Crossplay sessions
  - EOS Settings
    - Enable Overlay
    - Enable Social Overlay
3. Update `Config/DefaultEngine.ini` according to [the `Online Subsystem EOS` docs](https://docs.unrealengine.com/5.1/en-US/online-subsystem-eos-plugin-in-unreal-engine/).
4. Update `Config/DefaultGame.ini` to add the following:

```ini
[/Script/Engine.GameSession]
MaxPlayers=64
```


## Implementation

The following steps assume your project has both a default map and a lobby map already created. If not, create those maps as needed and ensure they're included in the packaged build of the project.

After installing and configuring the plugin, use the `WBP_DebugMenu` asset to test functionality:

1. Open the Level Blueprint for your project's Startup/Default Map.
2. Use the BeginPlay node to call `Create Widget`, creating a widget of type `WBP_DebugMenu`.
3. Use the Widget to call the `Add Multiplayer Debug Menu` function, providing the game mode and lobby map path.

Once this is done, opening your default map will show the multiplayer debug menu, which you can use to host or join sessions.

You can also use the included `DebugGameMode`, which will print player names and counts as they join or leave the session.

Additionally, the `DebugMenu` C++ class serves as a reference implementation of the plugin's functionality.
