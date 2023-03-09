# MultiplayerPlugin

Developed with Unreal Engine 5

## Polish Burndown List

- Finish all delegates and handlers
	- Better code organization
	- Better comments and documentation
- Expose all assumptions as configurables (Steam, result cap, etc.)
- Improve all error handling
	- Utility function to log to screen (GEngine) and text (ULOG)
	- Parameter to mark things as errors (GEngine red color, ULOG 'ERROR:' prefix)
- Polish and decouple debug menu where possible
- Port plugin to a new project/repo without added UE content