/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifdef _WIN32_WCE

// Browse directories to locate SCUMM games

#include "stdafx.h"
#include <assert.h>

#if _WIN32_WCE < 300

#include <Wingdi.h>
#include <Winbase.h>

#endif
#include <Winuser.h>
#include <Winnls.h>
#include "resource.h"
#include "scumm.h"
#include "config-file.h"
#include "screen.h"

extern Config *g_config;

#define MAX_GAMES 20
int MAX_DIRECTORY = 1;

#define MAX_DISPLAYED_DIRECTORIES 7
#define MAX_DISPLAYED_GAMES 7

struct ScummGame {
	const char *gamename;
	const char *description;
	const char *directory;
	const char *check_file_1;
	const char *check_file_2;
	const char *filename;
	unsigned char next_demo;
};

struct InstalledScummGame {
	unsigned char reference;
	TCHAR directory[MAX_PATH];
};

struct DirectoryName {
	TCHAR name[MAX_PATH];
};

struct GameReference {
	char name[100];
	unsigned char reference;
};

static const ScummGame GameList[] = {

	{	 "Simon 1 demo",
		 "Playable",
		 "0001.VGA", "GDEMO", "",
		 "simon1demo",
		 0
	},
	{	
		 "Simon 1 dos",
		 "Completable",
		 "", "1631.VGA", "GAMEPC",
		 "simon1dos",
		 0
	},
	{	 
		 "Simon 1 win",
		 "Completable",
		 "", "SIMON.GME", "GAMEPC",
		 "simon1win",
		 0
	},
	{	 
		 "Simon 2 dos",
		 "To be tested",
		 "", "SIMON2.GME", "GAME32",
		 "simon2dos",
		 0
	},
	{	 
		 "Simon 2 win",
		 "To be tested",
		 "", "SIMON2.GME", "GSPTR30",
		 "simon2win",
		 0
	},
	{ 
		 "Indiana Jones 3 VGA", 
	     "Buggy, playable a bit", 
		 "indy3", "", "", 
		 "indy3",
	     0 
	},
	{	 
		 "Zak Mc Kracken VGA",
		 "Completable",
		 "zak256", "", "",
		 "zak256",
		 0
	},
	{
		 "Loom old",
		 "Not working",
		 "loom", "", "",
		 "loom",
		 0
	},
	{
		 "Monkey Island 1 EGA",
		 "Not tested",
		 "monkeyEGA", "", "",
		 "monkeyEGA",
		 0
	},
	{
		 "Loom VGA",
		 "Completable, MP3 audio",
		 "loomcd", "", "",
		 "loomcd",
		 0
	},
	{
		 "Monkey Island 1 VGA CD",
		 "Completable, MP3 music",
		 "", "MONKEY.000", "MONKEY.001",
		 "monkey",
		 0
	},
	{
		 "Monkey Island 1 VGA CD",
		 "Completable, MP3 music",
		 "", "MONKEY1.000", "MONKEY1.001",
		 "monkey1",
		 0
	},
	{
		 "Monkey Island 1 VGA dk",
		 "Completable, no sound",
		 "monkey1vga", "", "",
		 "monkey1vga",
		 0
	},
	{
		 "Monkey Island 2 VGA",
		 "Completable",
		 "", "MONKEY2.000", "MONKEY2.001",
		 "monkey2",
		 0
	},
	{
		 "Indiana Jones 4",
		 "Completable",
		 "", "ATLANTIS.000", "ATLANTIS.001",
		 "atlantis",
		 1
	},
	{
		 "Indiana Jones 4 demo",
		 "Completable",
		 "", "PLAYFATE.000", "PLAYFATE.001",
		 "playfate",
		 0
	},
	{
		 "Day of the Tentacle",
		 "Completable",
		 "", "TENTACLE.000", "TENTACLE.001",
		 "tentacle",
		 1
	},
	{
		 "Day of Tentacle demo",
		 "Completable",
		 "", "DOTTDEMO.000", "DOTTDEMO.001",
		 "dottdemo",
		 0
	},
	{	
		 "Sam & Max",
		 "Completable",
		 "", "SAMNMAX.000", "SAMNMAX.001",
		 "samnmax",
		 1
	},
	{
		 "Sam & Max demo",
		 "Completable",
		 "", "SNMDEMO.000", "SNMDEMO.001",
		 "snmdemo",
		 0
	},
	{
		 "Full Throttle",
		 "Partially working",
		 "", "FT.LA0", "FT.LA1",
		 "ft",
		 0
	},
	{
		 "The Dig",
		 "Completable",
		 "", "DIG.LA0", "DIG.LA1",
		 "dig",
		 0
	},
	{
		 NULL, NULL, NULL, NULL, NULL, NULL, 0
	}
};

void findGame(TCHAR*);
int displayFoundGames(void);
void displayDirectoryList(void);
void displayGamesList(void);
void doScan(void);
void startFindGame(void);
bool loadGameSettings(void);

char gamesFound[MAX_GAMES];
GameReference listIndex[MAX_GAMES];
InstalledScummGame gamesInstalled[MAX_GAMES];
int installedGamesNumber;
HWND hwndDlg;
TCHAR basePath[MAX_PATH];
TCHAR old_basePath[MAX_PATH];
BOOL prescanning;

DirectoryName *directories = NULL;
int _first_index;
int _total_directories;
int _first_index_games;
int _total_games;
bool _scanning;
int _last_selected = -1;
bool _game_selected;

extern int _game_selection_X_offset;
extern int _game_selection_Y_offset;

int chooseGame(bool need_rescan) {

	if (directories)
		free(directories);

	directories = (DirectoryName*)malloc(MAX_DIRECTORY * sizeof(DirectoryName));

	setGameSelectionPalette();
	drawBlankGameSelection();
	
	if (!need_rescan)
		loadGameSettings();
	_game_selected = false;
	while (!_game_selected) {
		MSG msg;

		if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
			Sleep(100);
		else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}
	}

	return _last_selected;
}

/*
BOOL isPrescanning() {
	return prescanning;
}
*/

void setFindGameDlgHandle(HWND x) {
	hwndDlg = x;
}

bool loadGameSettings() {
	int				index;
	int				i;
	const char		*current;

	/*prescanning = FALSE;*/
	_scanning = false;

	current = g_config->get("GamesInstalled", "wince");
	if (!current)
		return FALSE;
	index = atoi(current);

	installedGamesNumber = index;

	current = g_config->get("GamesReferences", "wince");
	if (!current)
		return FALSE;
	for (i=0; i<index; i++) {
		char x[6];
		int j;

		memset(x, 0, sizeof(x));
		memcpy(x, current + 3 * i, 2);
		sscanf(x, "%x", &j);
		gamesFound[j] = 1;
		gamesInstalled[i].reference = j;
	}

	current = g_config->get("BasePath", "wince");
	if (!current)
		return FALSE;
	MultiByteToWideChar(CP_ACP, 0, current, strlen(current) + 1, basePath, sizeof(basePath));

	for (i=0; i<index; i++) {
		char keyName[100];

		sprintf(keyName, "GamesDirectory%d", i);
		current = g_config->get(keyName, "wince");
		if (!current)
			return FALSE;
		MultiByteToWideChar(CP_ACP, 0, current, strlen(current) + 1, gamesInstalled[i].directory, sizeof(gamesInstalled[i].directory));
	}

	displayFoundGames();

	return TRUE;
}

int countGameReferenced(int reference, int *infos) {
	int i;
	int number = 0;

	for (i=0; i<installedGamesNumber; i++)
		if (gamesInstalled[i].reference == reference) 
			infos[number++] = i;

	return number;
}

int displayFoundGames() {

	int i;	

	_total_games = 0;
	_first_index_games = 0;

	for (i = 0; i< MAX_GAMES; i++) {
		ScummGame current_game;
		char    work[400];
		//TCHAR	desc[400];
		int		numberReferenced;
		int		infos[10];
		int		j;

		current_game = GameList[i];
		if (!current_game.filename)
			break;
		if (!gamesFound[i])
			continue;
		
		numberReferenced = countGameReferenced(i, infos);

		for (j=0; j<numberReferenced; j++) {
			if (numberReferenced != 1)
				sprintf(work, "%s (%d)", current_game.gamename, j + 1);
			else
				strcpy(work, current_game.gamename);
			//MultiByteToWideChar(CP_ACP, 0, work, strlen(work) + 1, desc, sizeof(desc));
			//SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_ADDSTRING, 0, (LPARAM)desc);
			strcpy(listIndex[_total_games].name, work);
			listIndex[_total_games].reference = infos[j];
			_total_games++;
		}
	}

	displayGamesList();

	return _total_games;

}

void changeSelectedGame(int index) {

	int array_index;
	ScummGame game;

	if (index > MAX_DISPLAYED_GAMES - 1)
		return;

	if (_first_index_games)
		array_index = _first_index_games + index - 1;
	else
		array_index = index;

	if (array_index >= _total_games)
		return;

	// See if it's a previous/next link
	if (index == 0) {
		// potential previous
		if (_first_index_games) {
			_first_index_games -= (MAX_DISPLAYED_GAMES - 2);
			if (_first_index_games < 0 || _first_index_games - (MAX_DISPLAYED_GAMES - 2) < 0)
				_first_index_games = 0;
			resetLastHighlighted();
			displayGamesList();
			return;
		}
	}
	if (index == MAX_DISPLAYED_GAMES - 1) {
		char work[100];

		sprintf(work, "X %d Total %d", array_index, _total_games - 1);
		drawCommentString(work);
		// potential next
		if (array_index != _total_games) {
			if (!_first_index_games)
				_first_index_games += (MAX_DISPLAYED_GAMES - 1);
			else
				_first_index_games += (MAX_DISPLAYED_GAMES - 2);
			resetLastHighlighted();
			displayGamesList();
			return;
		}
	}

	// Highlight the current game and displays game informations

	drawHighlightedString(listIndex[array_index].name, index);	

	game = GameList[gamesInstalled[listIndex[array_index].reference].reference];
	drawCommentString((char*)game.description);

	_last_selected = array_index;
}
	

void changeScanPath(int index) {
	//int item;
	TCHAR path[MAX_PATH];
	int array_index;

	if (index > MAX_DISPLAYED_DIRECTORIES - 1)
		return;

	if (_first_index)
		array_index = _first_index + index - 1;
	else
		array_index = index;

	if (array_index >= _total_directories) 
		return;

	// See if it's a previous/next link
	if (index == 0) {
		// potential previous
		if (_first_index) {
			_first_index -= (MAX_DISPLAYED_DIRECTORIES - 2);
			if (_first_index < 0 || _first_index - (MAX_DISPLAYED_DIRECTORIES - 2) < 0)
				_first_index = 0;
			resetLastHighlighted();
			displayDirectoryList();
			return;
		}
	}
	if (index == MAX_DISPLAYED_DIRECTORIES - 1) {
		// potential next
		if (array_index != _total_directories) {
			if (!_first_index)
				_first_index += (MAX_DISPLAYED_DIRECTORIES - 1);
			else
				_first_index += (MAX_DISPLAYED_DIRECTORIES - 2);
			resetLastHighlighted();
			displayDirectoryList();
			return;
		}
	}

	wcscpy(path, directories[array_index].name);

	if (wcscmp(path, TEXT("..")) != 0) {
		wcscat(basePath, TEXT("\\"));
		wcscat(basePath, path);
	}
	else {
		TCHAR *work;
		
		work = wcsrchr(basePath, '\\');
		*work = 0;
		*(work + 1) = 0;
	}

	doScan();
}

void doScan() {
	WIN32_FIND_DATA	 desc;
	TCHAR			 searchPath[MAX_PATH];
	HANDLE			 x;

	//SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);	

	_total_directories = 0;
	_first_index = 0;

	if (wcslen(basePath) != 0) {
		//SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_ADDSTRING, 0, (LPARAM)TEXT(".."));
		if (_total_directories == MAX_DIRECTORY) {
			MAX_DIRECTORY += 1;
			directories = (DirectoryName*)realloc(directories, MAX_DIRECTORY  * sizeof(DirectoryName));
		}
		wcscpy(directories[_total_directories++].name, TEXT(".."));
	}

	wsprintf(searchPath, TEXT("%s\\*"), basePath);

	x = FindFirstFile(searchPath, &desc);
	if (x == INVALID_HANDLE_VALUE) {
		FindClose(x);
		displayDirectoryList();
		return;
	}
	if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		TCHAR *work;

		work = wcsrchr(desc.cFileName, '\\');
		/*
		SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), 
			LB_ADDSTRING, 0, (LPARAM)(work ? work + 1 : desc.cFileName));
		*/
		if (_total_directories == MAX_DIRECTORY) {
			MAX_DIRECTORY += 1;
			directories = (DirectoryName*)realloc(directories, MAX_DIRECTORY  * sizeof(DirectoryName));
		}
		wcscpy(directories[_total_directories++].name, (work ? work + 1 : desc.cFileName));
	}
	while (FindNextFile(x, &desc))
		if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		TCHAR *work;

		work = wcsrchr(desc.cFileName, '\\');
		/*
		SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), 
			LB_ADDSTRING, 0, (LPARAM)(work ? work + 1 : desc.cFileName));
		*/
		if (_total_directories == MAX_DIRECTORY) {
			MAX_DIRECTORY += 1;
			directories = (DirectoryName*)realloc(directories, MAX_DIRECTORY  * sizeof(DirectoryName));
		}
		wcscpy(directories[_total_directories++].name, (work ? work + 1 : desc.cFileName));
	}	

	FindClose(x);
	displayDirectoryList();
}

void displayGamesList() {
	int current = _first_index_games;
	int index_games = 0;

	_last_selected = -1;

	drawBlankGameSelection();
	drawCommentString("Choose a game");

	if (_first_index_games) {
		// include "previous" link
		drawStandardString("***[PREVIOUS]***", index_games++);
	}

	while (current < _total_games && index_games < MAX_DISPLAYED_GAMES - 1) 
		drawStandardString(listIndex[current++].name, index_games++);


	if (current != _total_games) {
		// add "next" link
		drawStandardString("***[NEXT]***", index_games);
	}
}

void displayDirectoryList() {
	int current = _first_index;	
	int index_link = 0;

	_last_selected = -1;

	drawBlankGameSelection();
	drawCommentString("Choose root directory");

	if (_first_index) {
		// include "previous" link
		drawStandardString("***[PREVIOUS]***", index_link++);
	}

	while (current < _total_directories && index_link < MAX_DISPLAYED_DIRECTORIES - 1) {
		char work[MAX_PATH];

		WideCharToMultiByte(CP_ACP, 0, directories[current].name, wcslen(directories[current].name) + 1, work, sizeof(work), NULL, NULL);
		if (strlen(work) > 17) {
			strcpy(work + 17, "[...]");
		}
		drawStandardString(work, index_link++);
		current++;
	}

	if (current != _total_directories) {
		// add "next" link
		drawStandardString("***[NEXT]***", index_link);
	}
}

void startScan() {
	/*prescanning = TRUE;*/
	_scanning = true;
	wcscpy(old_basePath, basePath);
	//SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Choose the games root directory"));
	drawCommentString("Choose root directory");
	/*
	SetDlgItemText(hwndDlg, IDC_SCAN, TEXT("OK"));
	SetDlgItemText(hwndDlg, IDC_GAMEDESC, TEXT(""));
	ShowWindow(GetDlgItem(hwndDlg, IDC_PLAY), SW_HIDE);
	*/
	doScan();
}

void endScanPath() {
	/*prescanning = FALSE;*/
	_scanning = false;
	/*
	SetDlgItemText(hwndDlg, IDC_SCAN, TEXT("Scan"));
	ShowWindow(GetDlgItem(hwndDlg, IDC_PLAY), SW_SHOW);
	*/
	startFindGame();
}

void abortScanPath() {
	/*prescanning = FALSE;*/
	_scanning = false;
	wcscpy(basePath, old_basePath);
	//SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT(""));
	drawCommentString("");
	/*
	SetDlgItemText(hwndDlg, IDC_SCAN, TEXT("Scan"));	
	SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);
	ShowWindow(GetDlgItem(hwndDlg, IDC_PLAY), SW_SHOW);
	*/
	displayFoundGames();
}

void startFindGame() {
	//TCHAR			fileName[MAX_PATH];
	//TCHAR			*tempo;
	int				i = 0;
	int		    	index = 0;
	char			tempo[1024];
	char		    workdir[MAX_PATH];	

	//prescanning = FALSE;
	_scanning = false;

	//SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Scanning, please wait"));
	drawBlankGameSelection();
	drawCommentString("Scanning, please wait");

	//SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_RESETCONTENT, 0, 0);

	memset(gamesFound, 0, MAX_GAMES);
	/*
	GetModuleFileName(NULL, fileName, MAX_PATH);
	tempo = wcsrchr(fileName, '\\');
	*tempo = '\0';
	*(tempo + 1) = '\0';
	*/
	installedGamesNumber = 0;

	//findGame(fileName);
	findGame(basePath);

	// Display the results
	index = displayFoundGames();

	// Save the results in the registry
	//SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Saving the results"));
	drawCommentString("Saving the results");

	g_config->setInt("GamesInstalled", index, "wince");

	tempo[0] = '\0';
	for (i=0; i<index; i++) {
		char x[3];
		sprintf(x, "%.2x ", gamesInstalled[i].reference);
		strcat(tempo, x);
	}	

	g_config->set("GamesReferences", tempo, "wince");

	WideCharToMultiByte(CP_ACP, 0, basePath, wcslen(basePath) + 1, workdir, sizeof(workdir), NULL, NULL);

	g_config->set("BasePath", workdir, "wince");

	for (i=0; i<index; i++) {
		char keyName[100];

		sprintf(keyName, "GamesDirectory%d", i);
		WideCharToMultiByte(CP_ACP, 0, gamesInstalled[i].directory, wcslen(gamesInstalled[i].directory) + 1, workdir, sizeof(workdir), NULL, NULL);
		g_config->set(keyName, workdir, "wince");
	}

	g_config->flush();

	//SetDlgItemText(hwndDlg, IDC_FILEPATH, TEXT("Scan finished"));
	drawCommentString("Scan finished");

}

void getSelectedGame(int result, char *id, TCHAR *directory) {
	ScummGame game;

	game = GameList[gamesInstalled[listIndex[result].reference].reference];
	strcpy(id, game.filename);
	wcscpy(directory, gamesInstalled[listIndex[result].reference].directory);
}

void displayGameInfo(int index) {
	//int item;	
	//TCHAR work[400];
	ScummGame game;

	/*
	item = SendMessage(GetDlgItem(hwndDlg, IDC_LISTAVAILABLE), LB_GETCURSEL, 0, 0);
	if (item == LB_ERR)
		return;
	*/

	game = GameList[gamesInstalled[listIndex[_last_selected].reference].reference];
	/*
	wcscpy(work, TEXT("File path : ..."));	
	wcscat(work, wcsrchr(gamesInstalled[listIndex[item]].directory, '\\'));			
	SetDlgItemText(hwndDlg, IDC_FILEPATH, work);
	MultiByteToWideChar(CP_ACP, 0, game.description, strlen(game.description) + 1, work, sizeof(work));
	SetDlgItemText(hwndDlg, IDC_GAMEDESC, work);	
	*/
	drawCommentString((char*)game.description);
}

void findGame(TCHAR *directory) {
	TCHAR			 fileName[MAX_PATH];
	TCHAR			 newDirectory[MAX_PATH];
	WIN32_FIND_DATA	 desc;
	HANDLE			 x;
	int				 i;

	// Check for games in the current directory

	//MessageBox(NULL, directory, TEXT("Current"), MB_OK);

	for (i = 0 ; i < MAX_GAMES ; i++) {
		ScummGame current_game;

		current_game = GameList[i];
		if (!current_game.filename)
			break;

		if (strlen(current_game.directory)) {
			// see if the last directory matches
			TCHAR	*work;
			char	curdir[MAX_PATH];

			
			work = wcsrchr(directory, '\\');
			WideCharToMultiByte(CP_ACP, 0, work + 1, wcslen(work + 1) + 1, curdir, sizeof(curdir), NULL, NULL);
			if (stricmp(curdir, current_game.directory) == 0) {
				
				//MessageBox(NULL, TEXT("Match directory !"), TEXT("..."), MB_OK);
				
				gamesFound[i] = 1;
				gamesInstalled[installedGamesNumber].reference = i;
				wcscpy(gamesInstalled[installedGamesNumber].directory, directory);
				installedGamesNumber++;
			}
		}
		else
		{
			TCHAR	work[MAX_PATH];
			TCHAR	checkfile[MAX_PATH];


			MultiByteToWideChar(CP_ACP, 0, current_game.check_file_1, strlen(current_game.check_file_1) + 1, checkfile, sizeof(checkfile));
			wsprintf(work, TEXT("%s\\%s"), directory, checkfile);
			//MessageBox(NULL, work, TEXT("Checking file"), MB_OK);

			if (GetFileAttributes(work) == 0xFFFFFFFF)
				continue;

			//MessageBox(NULL, TEXT("Check OK"), TEXT("Checking file"), MB_OK);
			MultiByteToWideChar(CP_ACP, 0, current_game.check_file_2, strlen(current_game.check_file_2) + 1, checkfile, sizeof(checkfile));
			wsprintf(work, TEXT("%s\\%s"), directory, checkfile);			
			if (GetFileAttributes(work) == 0xFFFFFFFF)
				continue;
			
			//MessageBox(NULL, TEXT("Match file !"), TEXT("..."), MB_OK);
			gamesFound[i] = 1;
			gamesInstalled[installedGamesNumber].reference = i;
			wcscpy(gamesInstalled[installedGamesNumber].directory, directory);
			installedGamesNumber++;

		}
	}

	// Recurse

	wsprintf(fileName, TEXT("%s\\*"), directory);

	x = FindFirstFile(fileName, &desc);
	if (x == INVALID_HANDLE_VALUE)
		return;
	if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		wsprintf(newDirectory, TEXT("%s\\%s"), directory, desc.cFileName);
		findGame(newDirectory);
	}
	while (FindNextFile(x, &desc))
		if (desc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		wsprintf(newDirectory, TEXT("%s\\%s"), directory, desc.cFileName);
		findGame(newDirectory);
	}	
	FindClose(x);
}

void handleSelectGameDown() {
	if (!_scanning) {
		if (_last_selected == -1) {
			if (_first_index_games)
				changeSelectedGame(1);
			else
				changeSelectedGame(0);
		}
		else
			changeSelectedGame(_last_selected - _first_index_games + 1);
	}
}

void handleSelectGameUp() {
	if (!_scanning) {
		if (_last_selected != -1 && _last_selected) {
			if (_last_selected == _first_index_games)
				changeSelectedGame(0);
			else
				changeSelectedGame(_last_selected - _first_index_games - 1);
		}
	}
}

void handleSelectGameButton() {
	if (!_scanning) {
		if (_last_selected == -1)
			drawCommentString("Please select a game");
		else
			_game_selected = true;
	}
}

void handleSelectGame(int x, int y) {

	int start = 70;
	int i;

	x -= _game_selection_X_offset;
	y -= _game_selection_Y_offset;

	if (y < start - 10) {
		drawVideoDevice();
		return;
	}

	/* See if it's a selection */

	for (i=0; i<MAX_DISPLAYED_GAMES; i++) {
		if (y >= start && y <= start + 8) {
			if (!_scanning)
				changeSelectedGame(i);
			else
				changeScanPath(i);
			return;
		}
		start += 15;
	}

	/* See if it's a button */

	if (y>=217 && y<=238) {
		if (x>=8 && x<=45) {
			if (!_scanning)
				startScan();
			else
				startFindGame();
		}
		if ((x>=93 && x<=129)) {
			if (!_scanning) {
				if (_last_selected == -1)
					drawCommentString("Please select a game");
				else
					_game_selected = true;
			}
			else
				startFindGame();
		}
		if (x>=175 && x<=208) {
			if (!_scanning) {
				_game_selected = true;
				_last_selected = -1;
			}
			else
				abortScanPath();
		}
	}
}			

#endif