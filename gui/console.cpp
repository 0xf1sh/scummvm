/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "console.h"
#include "ScrollBarWidget.h"

#include "common/engine.h"

/*

 _   _           _                                 _                   _   _             
| | | |_ __   __| | ___ _ __    ___ ___  _ __  ___| |_ _ __ _   _  ___| |_(_) ___  _ __  
| | | | '_ \ / _` |/ _ \ '__|  / __/ _ \| '_ \/ __| __| '__| | | |/ __| __| |/ _ \| '_ \ 
| |_| | | | | (_| |  __/ |    | (_| (_) | | | \__ \ |_| |  | |_| | (__| |_| | (_) | | | |
 \___/|_| |_|\__,_|\___|_|     \___\___/|_| |_|___/\__|_|   \__,_|\___|\__|_|\___/|_| |_|
                                                                                         
This code is not finished, so please don't complain :-)

*/


#define PROMPT	") "

/* TODO:
 * - it is very inefficient to redraw the full thingy when just one char is added/removed.
 *   Instead, we could just copy the GFX of the blank console (i.e. after the transparent
 *   background is drawn, before any text is drawn). Then using that, it becomes trivial
 *   to erase a single character, do scrolling etc.
 * - add a scrollbar widget to allow scrolling in the history
 * - a *lot* of others things, this code is in no way complete and heavily under progress
 */
ConsoleDialog::ConsoleDialog(NewGui *gui)
	: Dialog(gui, 0, 0, 320, 12*kLineHeight+2)
{
	_lineWidth = (_w - kScrollBarWidth - 2) / kCharWidth;
	_linesPerPage = (_h - 2) / kLineHeight;

	memset(_buffer, ' ', kBufferSize);
	_linesInBuffer = kBufferSize / _lineWidth;
	
	_currentPos = 0;
	_scrollLine = _linesPerPage - 1;
	
	_caretVisible = false;
	_caretTime = 0;
	
	// Add scrollbar
	_scrollBar = new ScrollBarWidget(this, _w - kScrollBarWidth - 1, 0, kScrollBarWidth, _h);
	_scrollBar->setTarget(this);

	// Display greetings & prompt
	print("ScummVM "SCUMMVM_VERSION" (" SCUMMVM_CVS ")\n");
	print("Console is ready\n");
	
	_promptStartPos = _promptEndPos = -1;
	
	// Init callback
	_callbackProc = 0;
	_callbackRefCon = 0;
 
 	// Init History
 	_historyIndex = 0;
 	_historyLine = 0;
 	_historySize = 0;
 	for (int i = 0; i < kHistorySize; i++)
 		_history[i][0] = '\0';
}

void ConsoleDialog::open()
{
	Dialog::open();
	if (_promptStartPos == -1) {
		print(PROMPT);
		_promptStartPos = _promptEndPos = _currentPos;
	}
}

void ConsoleDialog::drawDialog()
{
	// Blend over the background
	_gui->blendRect(_x, _y, _w, _h, _gui->_bgcolor, 2);
	
	// Draw a border
	_gui->hline(_x, _y+_h-1, _x+_w-1, _gui->_color);

	// Draw text
	int start = _scrollLine - _linesPerPage + 1;
	int y = _y + 2;
	for (int line = 0; line < _linesPerPage; line++) {
		int x = _x + 1;
		for (int column = 0; column < _lineWidth; column++) {
			int l = (start+line) % _linesInBuffer;
			byte c = _buffer[l * _lineWidth + column];
			_gui->drawChar(c, x, y, _gui->_textcolor);
			x += kCharWidth;
		}
		y += kLineHeight;
	}

	// Draw the scrollbar
	_scrollBar->draw();

	// Finally blit it all to the screen
	_gui->addDirtyRect(_x, _y, _w, _h);
}

void ConsoleDialog::handleTickle()
{
	uint32 time = _gui->get_time();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		if (_caretVisible) {
			drawCaret(true);
		} else {
			drawCaret(false);
		}
	}
}

void ConsoleDialog::handleMouseWheel(int x, int y, int direction)
{
	_scrollBar->handleMouseWheel(x, y, direction);
}

void ConsoleDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers)
{
	int i;
	
	switch (keycode) {
		case '\n':	// enter/return
		case '\r': {
			if (_caretVisible)
				drawCaret(true);
			
			nextLine();

			int len = _promptEndPos - _promptStartPos;
//			char str[len + 1];
			char str[1000];

			if (len < 0) len = 0;	// Prevent overflow from forced Ctrl-D deletion

			for (i = 0; i < len; i++)
				str[i] = _buffer[(_promptStartPos + i) % kBufferSize];
			str[len] = '\0';
			
 			addToHistory(str);

			bool keepRunning = true;
			if (_callbackProc)
				keepRunning = (*_callbackProc)(this, str, _callbackRefCon);
			//printf("You entered '%s'\n", str);

			print(PROMPT);
			_promptStartPos = _promptEndPos = _currentPos;
			
			draw();
			if (!keepRunning)
				close();
			break;
			}
		case 27:	// escape
			close();
			break;
		case 8:		// backspace
			if (_caretVisible)
				drawCaret(true);
		
			if (_currentPos > _promptStartPos) {
				_currentPos--;
				killChar();
			}
			scrollToCurrent();
			draw();	// FIXME - not nice to redraw the full console just for one char!
			break;
		case 127:
			killChar();
			draw();
			break;
/*
		case 256+24:	// pageup
			_selectedItem -= _entriesPerPage - 1;
			if (_selectedItem < 0)
				_selectedItem = 0;
			break;
		case 256+25:	// pagedown
			_selectedItem += _entriesPerPage - 1;
			if (_selectedItem >= _list.size() )
				_selectedItem = _list.size() - 1;
			break;
*/
		case 256+22:	// home
			_scrollLine = _linesPerPage - 1;	// FIXME - this is not correct after a wrap around
			updateScrollBar();
			draw();
			break;
		case 256+23:	// end
			_scrollLine = _currentPos / _lineWidth;
			updateScrollBar();
			draw();
			break;
		case 273:	// cursor up
			historyScroll(+1);
			break;
		case 274:	// cursor down
			historyScroll(-1);
			break;
		case 275:	// cursor right
			if (_currentPos < _promptEndPos)
				_currentPos++;
			draw();
			break;
		case 276:	// cursor left
			if (_currentPos > _promptStartPos)
				_currentPos--;
			draw();
			break;

		default:
			if (ascii == '~' || ascii == '#') {
				close();
			} else if (modifiers == OSystem::KBD_CTRL) {
				specialKeys(keycode);
			} else if (isprint((char)ascii)) {
				for (i = _promptEndPos-1; i >= _currentPos; i--)
					_buffer[(i+1) % kBufferSize] = _buffer[i % kBufferSize];
				_promptEndPos++;
				putchar((char)ascii);
				scrollToCurrent();
			}
	}
}

void ConsoleDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kSetPositionCmd:
		int newPos = (int)data + _linesPerPage - 1;
		if (newPos != _scrollLine) {
			_scrollLine = newPos;
			draw();
		}
		break;
	}
}

void ConsoleDialog::specialKeys(int keycode)
{
	switch (keycode) {
		case 'a':
			_currentPos = _promptStartPos;
			draw();
			break;
		case 'd':
			killChar();
			draw();
			break;
		case 'e':
			_currentPos = _promptEndPos;
			draw();
			break;
		case 'k':
			killLine();
			draw();
			break;
		case 'w':
			killLastWord();
			draw();
			break;
	}
}

void ConsoleDialog::killChar()
{
	for (int i = _currentPos; i < _promptEndPos; i++)
		_buffer[i % kBufferSize] = _buffer[(i+1) % kBufferSize];
	_buffer[_promptEndPos % kBufferSize] = ' ';
	_promptEndPos--;
}

void ConsoleDialog::killLine()
{
	for (int i = _currentPos; i < _promptEndPos; i++)
		_buffer[i % kBufferSize] = ' ';
	_promptEndPos = _currentPos;
}

void ConsoleDialog::killLastWord()
{
	int pos;
	int cnt = 0;
	while (_currentPos > _promptStartPos) {
		_currentPos--;
		pos = getBufferPos();
		if (_buffer[pos] != ' ')
			cnt++;
		else
			break;
	}

	for (int i = _currentPos; i < _promptEndPos; i++)
		_buffer[i % kBufferSize] = _buffer[(i+cnt+1) % kBufferSize];
	_buffer[_promptEndPos % kBufferSize] = ' ';
	_promptEndPos -= cnt + 1;
}

void ConsoleDialog::addToHistory(const char *str)
{
	strcpy(_history[_historyIndex], str);
	_historyIndex = (_historyIndex + 1) % kHistorySize;
	_historyLine = 0;
	if (_historySize < kHistorySize)
		_historySize++;
}

void ConsoleDialog::historyScroll(int direction)
{
	if (_historySize == 0)
		return;

	if (_historyLine == 0 && direction > 0) {
		int i;
		for (i = 0; i < _promptEndPos - _promptStartPos; i++)
			_history[_historyIndex][i] = _buffer[_promptStartPos + i];
		_history[_historyIndex][i] = '\0';
	}
	
	// Advance to the next line in the history
	int line = _historyLine + direction;
	if ((direction < 0 && line < 0) || (direction > 0 && line > _historySize))
		return;
	_historyLine = line;

	// Hide caret if visible
	if (_caretVisible)
		drawCaret(true);

	// Remove the current user text
	_currentPos = _promptStartPos;
	killLine();

	// ... and ensure the prompt is visible
	scrollToCurrent();

	// Print the text from the history
	int idx;
	if (_historyLine > 0)
		idx = (_historyIndex - _historyLine + _historySize) % _historySize;
	else
		idx = _historyIndex;
	for (int i = 0; i < kLineBufferSize && _history[idx][i] != '\0'; i++)
		putcharIntern(_history[idx][i]);
	_promptEndPos = _currentPos;
	
	// Ensure once more the caret is visible (in case of very long history entries)
	scrollToCurrent();
	
	draw();
}


void ConsoleDialog::nextLine()
{
	int line = _currentPos / _lineWidth;
	if (line == _scrollLine)
		_scrollLine++;
	_currentPos = (line + 1) * _lineWidth;
	
	updateScrollBar();
}

void ConsoleDialog::updateScrollBar()
{
	int line = _currentPos / _lineWidth;
	_scrollBar->_numEntries = (line < _linesInBuffer) ? line + 1 : _linesInBuffer;
	_scrollBar->_currentPos = _scrollBar->_numEntries - (line - _scrollLine + _linesPerPage);
	_scrollBar->_entriesPerPage = _linesPerPage;
	_scrollBar->recalc();
}

int ConsoleDialog::printf(const char *format, ...)
{
	va_list	argptr;

	va_start(argptr, format);
	int count = this->vprintf(format, argptr);
	va_end (argptr);
	return count;
}

int ConsoleDialog::vprintf(const char *format, va_list argptr)
{
	char	buf[2048];

#if defined(WIN32)
	int count = _vsnprintf(buf, sizeof(buf), format, argptr);
#else
	int count = vsnprintf(buf, sizeof(buf), format, argptr);
#endif
	print(buf);
	return count;
}

void ConsoleDialog::putchar(int c)
{
	if (_caretVisible)
		drawCaret(true);

	putcharIntern(c);

	draw();	// FIXME - not nice to redraw the full console just for one char!
}

void ConsoleDialog::putcharIntern(int c)
{
	if (c == '\n')
		nextLine();
	else {
		_buffer[getBufferPos()] = (char)c;
		_currentPos++;
		if ((_scrollLine + 1) * _lineWidth == _currentPos) {
			_scrollLine++;
			updateScrollBar();
		}
	}
}

void ConsoleDialog::print(const char *str)
{
	if (_caretVisible)
		drawCaret(true);

	while (*str)
		putcharIntern(*str++);

	draw();
}

void ConsoleDialog::drawCaret(bool erase)
{
	int line = _currentPos / _lineWidth;
	int displayLine = line - _scrollLine + _linesPerPage - 1;

	// Only draw caret if visible
	if (!isVisible() || displayLine < 0 || displayLine >= _linesPerPage) {
		_caretVisible = false;
		return;
	}

	int x = _x + 1 + (_currentPos % _lineWidth) * kCharWidth;
	int y = _y + displayLine * kLineHeight;

	char c = _buffer[getBufferPos()];
	if (erase) {
		_gui->fillRect(x, y, kCharWidth, kLineHeight, _gui->_bgcolor);
		_gui->drawChar(c, x, y+2, _gui->_textcolor);
	} else {
		_gui->fillRect(x, y, kCharWidth, kLineHeight, _gui->_textcolor);
		_gui->drawChar(c, x, y+2, _gui->_bgcolor);
	}
	_gui->addDirtyRect(x, y, kCharWidth, kLineHeight);
	
	_caretVisible = !erase;
}

void ConsoleDialog::scrollToCurrent()
{
	int line = _currentPos / _lineWidth;
	int displayLine = line - _scrollLine + _linesPerPage - 1;
	
	if (displayLine < 0) {
		// TODO - this should only occur for loong edit lines, though
	} else if (displayLine >= _linesPerPage) {
		_scrollLine = _currentPos / _lineWidth;
		updateScrollBar();
	}
}
