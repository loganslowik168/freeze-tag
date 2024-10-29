# CPE353 Team 15 Project - Freeze Tag
# Project Status
The project has been completed.

## Authors and Acknowledgment
- Logan Slowik
- Justina Edwards
- Jonah Davis
- Alex Cockrell
- Kentrell Martin

## Description
- This game uses a multicast network to connect up to 4 players on the Linux systems in ENG 246/246/216/blackhawk. 
- The game that will be played is Freeze Tag. The person that hosts the game will be the tagger and the remaining players will be chased by the tagger. 
- If the chased players remain unfrozen until the time runs out the tagger will lose and receive a score of 0. 
- If the players get tagged twice by the player they will receive 0 points. If they are all tagged once by the tagger they will receive points for the seconds they were not tagged.
- After the game ends the player's score will be displayed in a SQL database and their high scores will be displayed.
## How to Run
1. Download the files from the main branch in the gitlab as a .zip.
2. Unzip the files to the local directory.
3. Open the .pro file from the folder with Qt Creator.
4. Run the program.
5. Select an IP and port and choose a computer to host and up to 3 additional client to connect.
## Usage Example
1. Download source code from main as a .zip.
2. Unzip to a directory.
3. Navigate to the unzipped files in Qt Creator and open the .pro file.
4. Run the game (Ctrl + R)
5. Host must configure the connection before other players can join
6. If Host:
- Select the IPv4 Address and Multicast Port for players to join
- Enter username (optional)
- Select the Host Game button
- Pregame Chat should display: **Joined the Multicast Successfully!**
- Give the other users the IP address and Port to connect to the game
- When all the players are connected select the Start Button
7. If player:
- Enter username (optional)
- Connect to the IP and Port of the Host once the host is connected
- Pregame Chat should display: **Joined the Multicast Successfully!**
- When all players have joined the host can start the game
## Support
**Email:**
- Logan Slowik- lcs0026@uah.edu
- Justina Edwards- jle0025@uah.edu
- Jonah Davis- jpd0014@uah.edu
- Alex Cockrell - abc00042@uah.edu
- Kentrell Martin - km0159@uah.edu

## Technologies Used
- Qt Creator (Community)
