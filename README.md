# Chess3D
A 3D Chess game created with my custom game engine that can be played locally between friends or with a remote client.
Features Chess gameplay and movement, chess piece and board drawing, blinn phong shading and lighting, basic networking, saving and loading matches, and undo and redo.

![Runner Banner](https://github.com/jswilkinSMU/Chess3D/blob/main/Chess3DImage.png)

### How to Use:
	Keyboard Controls: 
		- WASD to move around FreeFly camera
		- Mouse cursor to look around with freefly camera.
		- Mouse left click on piece to select, right click to deselect
		- Hover over valid empty square with mouse and left click to move piece
		- Left arrow key to rewind one move. Undo
		- Right arrow key to forward one move. Redo

	- Debugging controls:
		- F2 to show raycasting debug visuals.
		- F4 to switch camera modes.
		
	- Shader debugging controls:
		- 1 to show only diffuse texture.
		- 2 to show only vertex color.
		- 3 to show uvs.
		- 7 to show only normal map.
		- 8 to show pixel normal TBN space.
		- 9 to show pixel normal world space.
		- T to show World Tangents.
		- B to show World Bitangents.
		- N to show World Normals.
		- L to show light intensity.
		- Numpad 0 to show only SGE texture.
		- Numpad 1 to show only Specular.
		- Numpad 2 to show only Glossiness.
		- Numpad 3 to show only Emissive.
		- Numpad 4 to show lightColor times specular.
		- Numpad 5 to show lightColor times texture color.
		- Numpad 6 to show amplified specularity.

### Chess Events:
	- ChessBegin: Starts a new chess match.
		- Execute with ChessBegin
	- ChessMove: Where most of the chess gameplay logic is handled.
		- Execute with ChessMove from=b2 to=b4
	- ChessServerInfo: Shows current server, port, connection status, and game state.
		- Execute with ChessServerInfo
	- ChessListen: Calls StartServer on this instance.
		- Execute with ChessListen
	- ChessConnect: Calls StartClient on this instance.
		- Execute with ChessConnect
	- ChessDisconnect: If server, disconnects self and clients. If client disconnects self.
		- Execute with ChessDisconnect reason="text"
	- ChessPlayerInfo: Writes and saves our player name and index.
		- Execute with ChessPlayerInfo player=0 name="name"
	- ChessValidate: Send after each command to validate the current match.
		- Execute with ChessValidate
	- ChessResign: Event to resign from the current match.
		- Execute with ChessResign player=0
	- ChessOfferDraw: Offer a draw/tie to the opponent.
		- Execute with ChessOfferDraw
	- ChessAcceptDraw: Accepts opponent's offered draw and finishes the match.
		- Execute with ChessAcceptDraw
	- ChessRejectDraw: Rejects opponent's offered draw and continues the match.
		- Execute with ChessRejectDraw
	- ChessSaveGame: Saves the current match to an xml file.
		- Execute with SaveGame file="filename.xml"
	- ChessLoadGame: Loads a chess match from an xml file.
		- Execute with LoadGame file="filename.xml"


### Build and Use:

	1. Download and Extract the zip folder.
	2. Open the Run folder.
	3. Double-click Chess3D_Release_x64.exe to start the program.
