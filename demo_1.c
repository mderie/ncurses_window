
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

FILE *fLog;

// Simulate a namespace...
#define NCURSES(cmd) cmd

#define WASTED_SCREEN_LINES 4
#define OUTPUT_LINES_MAX 255
char *outputLines[OUTPUT_LINES_MAX];
int outputLineCount;
int maxX, maxY;
char lastCommand[255] = "";

// Care : a "log" macro already exists...
void logThis(char *what)
{
  fLog = fopen("log.txt", "a");
  fprintf(fLog, "%s\n", what);
  fclose(fLog);
}

void scrollLastOutputLines()
{
  logThis("scrollLastOutputLines ==> Entering, outputLineCount = ...");
  // This is not 100 % portable
  //itoa(lastOutputY, snum, 10);
  char snum[9];
  sprintf(snum, "%d", outputLineCount);
  logThis(snum);

  if (outputLineCount == 0)
  {
    logThis("scrollLastOutputLines ==> Leaving : nothing to do !");
    return;
  }

  free(outputLines[0]); // up scroll :)
  for (int i = 0; i < outputLineCount; i++)
  {
    outputLines[i] = outputLines[i + 1];
  }
  outputLineCount--;
  logThis("scrollLastOutputLines ==> Leaving");
}

void clearLastOutputLines()
{
  logThis("clearLastOutputLines ==> Entering, outputLineCount = ...");
  char snum[9];
  sprintf(snum, "%d", outputLineCount);
  logThis(snum);

  if (outputLineCount == 0)
  {
    logThis("clearLastOutputLines ==> Leaving : nothing to do !");
    return;
  }

  for(int i = 0; i < outputLineCount; i++)
  {
    free(outputLines[i]);
  }
  outputLineCount = 0;
  logThis("clearLastOutputLines ==> Leaving");
}

void showLastOutputLines()
{
  logThis("showLastOutputLines ==> Entering, outputLineCount = ...");
  char snum[9];
  sprintf(snum, "%d", outputLineCount);
  logThis(snum);

  if (outputLineCount == 0)
  {
    logThis("showLastOutputLines ==> Leaving : nothing to do !");
    return;
  }

  int firstLineOffset = 0; // We start at the first line of the buffer
  // Except if we have more lines in the buffer than the screen
  if (outputLineCount > maxY - WASTED_SCREEN_LINES)
  {
    logThis("showLastOutputLines ==> Correcting firstLineOffset value");
    firstLineOffset = outputLineCount - maxY + WASTED_SCREEN_LINES;
  }

  logThis("showLastOutputLines ==> firstLineOffset = ...");
  sprintf(snum, "%d", firstLineOffset);
  logThis(snum);
  logThis("showLastOutputLines ==> maxY - WASTED_SCREEN_LINES = ...");
  sprintf(snum, "%d", maxY - WASTED_SCREEN_LINES);
  logThis(snum);

  for(int i = 1; i < maxY - WASTED_SCREEN_LINES + 1; i++)
  {
     logThis("showLastOutputLines ==> i + firstLineOffset - 1 = ...");
     sprintf(snum, "%d", i + firstLineOffset - 1);
     logThis(snum);
     logThis("showLastOutputLines ==> outputLines[i + firstLineOffset - 1] = ...");
     logThis(outputLines[i + firstLineOffset - 1]);
     mvwprintw(stdscr, i, 1, outputLines[i + firstLineOffset - 1]);

     if ((i + firstLineOffset - 1) == (outputLineCount - 1))
     {
       logThis("showLastOutputLines ==> Last line displayed");
        break;
     }
  }

 logThis("showLastOutputLines ==> Leaving");
}

void pushbackOutputLines(char* what)
{
  logThis("pushbackOutputLines ==> Entering, outputLineCount = ...");
  char snum[9];
  sprintf(snum, "%d", outputLineCount);
  logThis(snum);

  char *line = malloc(strlen(what));
  strcpy(line, what);
  outputLines[outputLineCount++] = line;

  if (outputLineCount == OUTPUT_LINES_MAX)
  {
     scrollLastOutputLines();
  }

  logThis("pushbackOutputLines ==> Leaving");
}

void rebuildScreen()
{
  logThis("rebuildScreen ==> Entering");
  getmaxyx(stdscr, maxY, maxX); // Get the screen height & width

  logThis("rebuildScreen ==> maxY = ...");
  char snum[9];
  sprintf(snum, "%d", maxY);
  logThis(snum);

  clear(); // We repaint all the screen each time !
      box(stdscr, ACS_VLINE, ACS_HLINE);
      wmove(stdscr, maxY - 3, 1);
      hline(ACS_HLINE, maxX - 2);
      showLastOutputLines();
      mvwprintw(stdscr, maxY - 2, 1, "USynth> ");
      refresh();
      logThis("rebuildScreen ==> Leaving : done !");
}

void ctor()
{
  initscr(); // Initialize ncurses
  cbreak(); // No line buffering
  keypad(stdscr, TRUE); // Allow extended key like F1
  curs_set(TRUE); // We want to see the cursor
  noecho();
  //scrollok(stdscr, TRUE); // Allow scolling content
  clearok(stdscr, TRUE); // Allow clear screen
}

void dtor()
{
  NCURSES(endwin)();
}

// TODO: Handle the HOME, END, UP, DOWN keys
// TODO: Trim the output buffer !

// Source https://stackoverflow.com/questions/5073045/ncurses-getstr-with-a-movable-cursor
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
void readline(char *buffer, int buflen)
{
  logThis("readline ==> Entering");

  move(maxY - 2, 9);
  int old_curs = curs_set(1);
  int pos = 0;
  int len = 0;
  int x, y;

  getyx(stdscr, y, x); // Get the current position...
  for (;;)
  {
    int c;

    buffer[len] = ' ';
    mvaddnstr(y, x, buffer, len + 1); // Print the buffer content
    move(y, x+pos);

    c = getch();

    if (c == KEY_ENTER || c == '\n' || c == '\r')
    {
      break;
    }
    else if (isprint(c))
    {
      if (pos < buflen - 1)
      {
        memmove(buffer + pos + 1, buffer + pos, len - pos);
        buffer[pos++] = c;
        len += 1;
      }
      else
      {
        beep();
      }
    }
    else if (c == KEY_LEFT)
    {
      if (pos > 0)
      {
        pos -= 1;
      }
      else
      {
       beep();
      }
    }
    else if (c == KEY_RIGHT)
    {
      if (pos < len)
      {
       pos += 1;
       }
       else
       {
        beep();
       }
    }
    else if (c == KEY_BACKSPACE)
    {
      if (pos > 0)
      {
        memmove(buffer + pos - 1, buffer + pos, len - pos);
        pos -= 1;
        len -= 1;
      }
      else
      {
        beep();
      }
    }
    else if (c == KEY_DC)
    {
      if (pos < len)
      {
        memmove(buffer + pos, buffer + pos + 1, len - pos - 1);
        len -= 1;
      }
      else
      {
        beep();
      }
    }
    else
    {
      beep();
    }
  } // for (;;)

  buffer[len] = '\0';
  if (old_curs != ERR)
  {
   for (int i = 0; i < len; i++)
   {
      mvaddch(y, x + i, ' ');
   }
   curs_set(old_curs);
  }

  // refresh();
  logThis("readline ==> Leaving : buffer = ...");
  logThis(buffer);
}

bool handleCommand(char* what)
{
    logThis("handleCommand ==> Entering what = ...");
    logThis(what);

    if (strcmp(what, "clear") == 0)
    {
      clearLastOutputLines();
      logThis("handleCommand ==> Leaving : clear");
      return true;
    }

    // Last possibility :)
    if ((strcmp(what, "exit") == 0) || (strcmp(what, "quit") == 0))
    {
      logThis("handleCommand ==> bye bye");
      memset(what, 0x00,strlen(what));
    }

    logThis("handleCommand ==> Leaving : error or leave");
    return false;
}

int main(int argc, char *argv[])
{
  char command[255] = "";
  outputLineCount = 0;
  ctor();

  while(true)
  {
    rebuildScreen();
    memset(command, 0x00, 255);
    logThis("main ==> About to readline : command = ''");
    readline(command, 255); // Blocking call !
    if (strlen(command) == 0)
    {
      continue;
    }

    if (!handleCommand(command))
    {
      logThis("main ==> Back in main loop, command = ...");
      logThis(command);
      if (strlen(command) > 0)
      {
        logThis("main ==> Unrecognized command");
        pushbackOutputLines(strcat(command, " : unrecognized command :("));
      }
      else
      {
         logThis("main ==> About to leave the main loop");
         break;
      }
    }
    else
    {
      logThis("main ==> Command well processed = ...");
      logThis(command);
    }
  }

  dtor();
  logThis("main ==> That's all folks");

  return 0;
}
