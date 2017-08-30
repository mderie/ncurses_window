
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

FILE *fLog;

// a log macro already exists...
void logThis(char *what)
{
  fLog = fopen("log.txt", "a");

  fprintf(fLog, "%s\n", what);

//  const int lf = 0x10;
//  fprintf(fLog, "%s", lf);

  fclose(fLog);
}

// To be replaced by the box command !!!
void drawBorders(WINDOW *screen)
{
  int x, y, i;

  refresh(); // Needed ?

  getmaxyx(screen, y, x);

  // 4 corners
  mvwprintw(screen, 0, 0, "+");
  mvwprintw(screen, y - 1, 0, "+");
  mvwprintw(screen, 0, x - 1, "+");
  mvwprintw(screen, y - 1, x - 1, "+");

  // Left & right sides
  for (i = 1; i < (y - 1); i++)
  {
    mvwprintw(screen, i, 0, "|");
    mvwprintw(screen, i, x - 1, "|");
  }

  // Top and bottom
  for (i = 1; i < (x - 1); i++)
  {
    mvwprintw(screen, 0, i, "-");
    mvwprintw(screen, y - 1, i, "-");
  }

  wrefresh(screen); // Needed ?
  refresh();
}

/*
void pushLines(std::vector<std::string> v)
{
//TODO: std::foreach(v::begin(),v::end(), pushline(iter))
}
*/

#define PROMPT_ZONE_HEIGHT 3

WINDOW *field;
WINDOW *score;
int parent_x;
int parent_y;

char lastCommand[300] = "";

void pushLine(char* what)
{
  logThis("pushLine ==> Entering what = ...");
  logThis(what);

  //mvwprintw(score, 1, 50, what);
  //wrefresh(field);
  //wrefresh(score);

  strcpy(lastCommand, what);

/*
    // Does not compile :(
    // lastCommand = "";
    memset(lastCommand, 0x00, 300);
    strcat(lastCommand, "Last command = ");
    strcat(lastCommand, what);
    mvwprintw(field, 1, 1, lastCommand); // Top
    //mvwprintw(score, 1, 1, "USynth> "); // Bottom
*/

    //TODO: scrolling here...
    logThis("pushLine ==> Leaving");
}

void flood()
{
 for (int i = 0; i < 50; i++)
 {
   pushLine("Ahu" + 0x0A + 0x00);
 }
}

//TODO : split this in two part... One that adjust the window size and that put back the content !
void rebuildScreen()
{
  logThis("rebuildScreen ==> Entering");
  int new_x, new_y;
  getmaxyx(stdscr, new_y, new_x);

  /*
  if ((new_y == parent_y) && (new_x == parent_x))
  {
    logThis("rebuildScreen ==> Leaving : nothing to do !");
    //refresh();
     return;
  }
  */
      parent_x = new_x;
      parent_y = new_y;

      wresize(field, new_y - PROMPT_ZONE_HEIGHT, new_x);

      wresize(score, PROMPT_ZONE_HEIGHT, new_x);
      mvwin(score, new_y - PROMPT_ZONE_HEIGHT, 0); // Strange...

      wclear(stdscr); // To be moved up ?
      refresh(); // No effect !!!!!

      wclear(field);
      //drawBorders(field);
      box(field, ACS_VLINE, ACS_HLINE);

      if (strlen(lastCommand) > 0)
      {
         logThis("rebuildScreen ==> lastCommand = ...");
         logThis(lastCommand);
         mvwprintw(field, 1, 1, lastCommand);
      }
      wrefresh(field);
      refresh();

      wclear(score);
      drawBorders(score);
      mvwprintw(score, 1, 1, "USynth> ");
      wrefresh(score);
      refresh();

      logThis("rebuildScreen ==> Leaving : done !");
}

void ctor()
{
  initscr(); // Initialize ncurses
  //echo();
  cbreak(); // No line buffering
  keypad(stdscr, TRUE); // Allow extended key like F1
  curs_set(TRUE); // We want to see the cursor
  noecho();
  //curs_set(FALSE);

  // set up initial windows
  getmaxyx(stdscr, parent_y, parent_x);

  // Source = https://stackoverflow.com/questions/19748685/curses-library-why-does-getch-clear-my-screen
  refresh();

  WINDOW *field = newwin(parent_y - PROMPT_ZONE_HEIGHT, parent_x, 0, 0);
  //drawBorders(field);
  box(field, ACS_VLINE, ACS_HLINE);
  wrefresh(field);

  WINDOW *score = newwin(PROMPT_ZONE_HEIGHT, parent_x, parent_y - PROMPT_ZONE_HEIGHT, 0);
  drawBorders(score);
  mvwprintw(score, 1, 1, "USynth> ");
  wrefresh(score);

  //usleep(50);
  refresh();
}

void dtor()
{
  endwin();
}

void waitForQuitOrExit()
{
}

// Source https://stackoverflow.com/questions/5073045/ncurses-getstr-with-a-movable-cursor
/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
void readline(char *buffer, int buflen) //, WINDOW *screen)
{
  logThis("readline ==> Entering");

  refresh(); // Needed ?

  int old_curs = curs_set(1);
  int pos = 0;
  int len = 0;
  int x, y;

  getyx(stdscr, y, x);
  for (;;)
  {
    int c;

    buffer[len] = ' ';
    mvaddnstr(y, x, buffer, len+1); // Print the buffer content
    move(y, x+pos);

    // Does not work at all
    //c = wgetch(score);
    c = getch(); // getch() = wgetch(screen); ==> So clears the whole screen
    rebuildScreen(); // Overkill after each key stroke ... But we are in text mode ;-)

    /*
    char *str = (char *) malloc(16);
    snprintf(str, 16, "%d", c);
    logThis(str);
    free(str);
    */

    //TODO : implement a swich statement
    //TODO : previous command should be available with UP & DOWN arrow keys

    if (c == KEY_ENTER || c == '\n' || c == '\r')
    {
      break;
    }
    else if (isprint(c))
    {
      if (pos < buflen-1)
      {
        memmove(buffer+pos+1, buffer+pos, len-pos);
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
        memmove(buffer+pos-1, buffer+pos, len-pos);
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
        memmove(buffer+pos, buffer+pos+1, len-pos-1);
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
   clrtoeol(); // doesn't work as expected ! We still need to clear the command chararcters !
   curs_set(old_curs);
  }

  //wrefresh(screen);
  //rebuildScreen();
  refresh();
  logThis("readline ==> Leaving : buffer = ...");
  logThis(buffer);
}

bool handleCommand(char* what)
{
   logThis("handleCommand ==> Entering what = ...");
   logThis(what);

    if (strcmp(what, "flood") == 0)
    {
      flood();
      logThis("handleCommand ==> Leaving : flood");
      return true;
    }

    // Or (re)draw, (re)build, (re)paint ...
    if (strcmp(what, "draw") == 0)
    {
      rebuildScreen();
      logThis("handleCommand ==> Leaving : draw");
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

  /*
  move(new_y - 2, 9);
  readline(command, 255);
  mvwprintw(field, 1, 1, command);

  endwin();
  refresh();
  sleep(2);

  return 0;
  */

  ctor();

  while(true)
  {
   //buildScreen();

    // relative coord doesn't work like this...
    //wmove(score, 1, 9);
    // Then let's go for absolute...
    memset(command, 0x00, 255);
    move(parent_y - 2, 9);
    logThis("main ==> About to readline : command = ''");
    readline(command, 255); // Blocking call !
    if (strlen(command) == 0)
    {
      continue;
    }

    //getnstr(command, 255);

    // refresh each window
    // wrefresh(field);
    // wrefresh(score);

    if (!handleCommand(command))
    {
      //rebuildScreen();
      logThis("main ==> Back in main loop, command = ...");
      logThis(command);
      if (strlen(command) > 0)
      {
        logThis("main ==> Unrecognized command");
        pushLine(strcat(command, " : unrecognized command :("));
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

  // Commenting the while (true) and allowing the delay shows the window contents !!!
  //sleep(3);
  dtor();

  logThis("main ==> That's all folks");

  return 0;
}
