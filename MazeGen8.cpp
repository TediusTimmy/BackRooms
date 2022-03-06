/*
Copyright (c) 2022 Thomas DiModica.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Thomas DiModica nor the names of other contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THOMAS DIMODICA AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THOMAS DIMODICA OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include <cstdint>

#include "JAVA.h"
#include "StaffordMix.h"

/*
   OOF:
      When you spend thirty minutes trying to track down a crash, only to realize that
      Cygwin GCC is LP64, but MinGW is LLP64.
*/
#include <stack>

const int MAX = 256;
const int TOP = MAX - 1;

enum EDGE
 {
   WALL,
   OPEN,
   DOOR,
   DOOR_UP,
   DOOR_DOWN,
   CLOSED_DOOR,
   LOCKED_DOOR,
   STAIRS
 };

class Cell
 {
   public:
      EDGE le, re, ue, de;
      bool v;

      Cell() : le(WALL), re(WALL), ue(WALL), de(WALL), v(false) { }
 };

class Board
 {
   public:
      unsigned int x, y, z, l;
      Cell cell [MAX][MAX];
 };

/*
   There is poor side behavior at (x, x + 2). The right and bottom sides will be the same.
   Theoretically, this behavior is intrinsic to the side-choosing use of Mix function.
   Door and standard RNG behavior look good.
   A lot of trial and error went into getting this to where it is at.
*/
uint64_t Mix(uint64_t a, uint64_t b, uint64_t c, uint64_t d)
 {
   a = harden(a);
   b = harden(b);
   c = harden(c);
   d = harden(d);
   return harden(harden(a - b) - harden(b - c) - harden(c - d));
 }

/*
   Use the most significant bits to generate the random value, as those will have the most entropy.
*/
int Rand(JAVA& rng, std::size_t max)
 {
   return (int)((rng.getNext() * max) >> 48);
 }

void addDoors(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 1024) + 1024;

   while (--n)
    {
      bool found = false;
      while (false == found)
       {
         int x = Rand(rng, MAX - 2) + 1;
         int y = Rand(rng, MAX - 2) + 1;
         int d = Rand(rng, 4);
         switch (d)
          {
         case 0:
            if (WALL == board.cell[y][x].le)
             {
               int type = Rand(rng, 4);
               board.cell[y][x].le = (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y][x - 1].re = (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
         case 1:
            if (WALL == board.cell[y][x].re)
             {
               int type = Rand(rng, 4);
               board.cell[y][x].re = (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y][x + 1].le = (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
         case 2:
            if (WALL == board.cell[y][x].ue)
             {
               int type = Rand(rng, 4);
               board.cell[y][x].ue = (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y - 1][x].de = (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
         case 3:
            if (WALL == board.cell[y][x].de)
             {
               int type = Rand(rng, 4);
               board.cell[y][x].de = (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y + 1][x].ue = (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
          }
       }
    }
 }

// Are all of the walls to be removed WALL, OPEN, or DOOR?
// At this point, the only other options are DOOR_UP or DOOR_DOWN.
bool checkOffice(Board& board, int x, int y, int w, int h)
 {
   // Only check down edges and right edges: there should be symmetry.
   for (int cx = x; cx < x + w; ++cx)
      for (int cy = y; cy < y + h; ++cy)
       {
         if (((cx + 1) != (x + w)) && ((DOOR_UP == board.cell[cy][cx].re) || (DOOR_DOWN == board.cell[cy][cx].re)))
            return false;
         if (((cy + 1) != (y + h)) && ((DOOR_UP == board.cell[cy][cx].de) || (DOOR_DOWN == board.cell[cy][cx].de)))
            return false;
         if (true == board.cell[cy][cx].v)
            return false;
       }
   return true;
 }

void setOffice(Board& board, int x, int y, int w, int h)
 {
   for (int cx = x; cx < x + w; ++cx)
      for (int cy = y; cy < y + h; ++cy)
       {
         if (cx != x)
            board.cell[cy][cx].le = OPEN;
         if ((cx + 1) != (x + w))
            board.cell[cy][cx].re = OPEN;
         if (cy != y)
            board.cell[cy][cx].ue = OPEN;
         if ((cy + 1) != (y + h))
            board.cell[cy][cx].de = OPEN;
         board.cell[cy][cx].v = true;
       }
 }

void addOffices(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 96) + 128;

   while (--n)
    {
      bool found = false;
      while (false == found)
       {
         int x = Rand(rng, MAX - 9) + 1;
         int y = Rand(rng, MAX - 9) + 1;
         int w = Rand(rng, 16) + 8;
         int h = Rand(rng, 16) + 8;
         if (x + w > (TOP - 1))
            w = TOP - 1 - x;
         if (y + h > (TOP - 1))
            h = TOP - 1 - y;
         if (true == checkOffice(board, x, y, w, h))
          {
            setOffice(board, x, y, w, h);
            found = true;

            int n = Rand(rng, w);
            if (WALL == board.cell[y][x + n].ue)
             {
               EDGE type = Rand(rng, 8) ? OPEN : DOOR;
               board.cell[y][x + n].ue = type;
               board.cell[y - 1][x + n].de = type;
             }
            n = Rand(rng, h);
            if (WALL == board.cell[y + n][x + w - 1].re)
             {
               EDGE type = Rand(rng, 8) ? OPEN : DOOR;
               board.cell[y + n][x + w - 1].re = type;
               board.cell[y + n][x + w].le = type;
             }
            n = Rand(rng, w);
            if (WALL == board.cell[y + h - 1][x + n].de)
             {
                  EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                  board.cell[y + h - 1][x + n].de = type;
                  board.cell[y + h][x + n].ue = type;
             }
            n = Rand(rng, h);
            if (WALL == board.cell[y + n][x].le)
             {
               EDGE type = Rand(rng, 8) ? OPEN : DOOR;
               board.cell[y + n][x].le = type;
               board.cell[y + n][x - 1].re = type;
             }
          }
       }
    }
 }

void spaceFill(JAVA& rng, Board& board, int x, int y)
 {
   bool fillFill = false;
   if (-1 == x)
    {
      fillFill = true;
      do
       {
         x = Rand(rng, MAX);
         y = Rand(rng, MAX);
       }
      while (true == board.cell[y][x].v);
    }
   board.cell[y][x].v = true;

   std::stack<int> xs;
   std::stack<int> ys;

   xs.push(x);
   ys.push(y);

   while (false == xs.empty())
    {
      int n = 0;
      if ((x - 1 >  -1) && (false == board.cell[y][x - 1].v) && (WALL == board.cell[y][x].le)) ++n;
      if ((x + 1 < MAX) && (false == board.cell[y][x + 1].v) && (WALL == board.cell[y][x].re)) ++n;
      if ((y - 1 >  -1) && (false == board.cell[y - 1][x].v) && (WALL == board.cell[y][x].ue)) ++n;
      if ((y + 1 < MAX) && (false == board.cell[y + 1][x].v) && (WALL == board.cell[y][x].de)) ++n;

      while (n)
       {
         bool found = false;
         while (false == found)
          {
            int d = Rand(rng, 4);
            switch (d)
             {
            case 0:
               if ((x - 1 >  -1) && (false == board.cell[y][x - 1].v) && (WALL == board.cell[y][x].le))
                {
                  EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                  board.cell[y][x].le = type;
                  board.cell[y][x - 1].re = type;
                  board.cell[y][x - 1].v = true;
                  xs.push(x);
                  ys.push(y);
                  x = x - 1;
                  found = true;
                }
               break;
            case 1:
               if ((x + 1 < MAX) && (false == board.cell[y][x + 1].v) && (WALL == board.cell[y][x].re))
                {
                  EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                  board.cell[y][x].re = type;
                  board.cell[y][x + 1].le = type;
                  board.cell[y][x + 1].v = true;
                  xs.push(x);
                  ys.push(y);
                  x = x + 1;
                  found = true;
                }
               break;
            case 2:
               if ((y - 1 >  -1) && (false == board.cell[y - 1][x].v) && (WALL == board.cell[y][x].ue))
                {
                  EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                  board.cell[y][x].ue = type;
                  board.cell[y - 1][x].de = type;
                  board.cell[y - 1][x].v = true;
                  xs.push(x);
                  ys.push(y);
                  y = y - 1;
                  found = true;
                }
               break;
            case 3:
               if ((y + 1 < MAX) && (false == board.cell[y + 1][x].v) && (WALL == board.cell[y][x].de))
                {
                  EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                  board.cell[y][x].de = type;
                  board.cell[y + 1][x].ue = type;
                  board.cell[y + 1][x].v = true;
                  xs.push(x);
                  ys.push(y);
                  y = y + 1;
                  found = true;
                }
               break;
             }
          }

         n = 0;
         if ((x - 1 >  -1) && (false == board.cell[y][x - 1].v) && (WALL == board.cell[y][x].le)) ++n;
         if ((x + 1 < MAX) && (false == board.cell[y][x + 1].v) && (WALL == board.cell[y][x].re)) ++n;
         if ((y - 1 >  -1) && (false == board.cell[y - 1][x].v) && (WALL == board.cell[y][x].ue)) ++n;
         if ((y + 1 < MAX) && (false == board.cell[y + 1][x].v) && (WALL == board.cell[y][x].de)) ++n;
       }

      x = xs.top();
      y = ys.top();
      xs.pop();
      ys.pop();
    }

   if (true == fillFill)
      for (y = 0; y < MAX; ++y)
         for (x = 0; x < MAX; ++x)
            if (false == board.cell[y][x].v)
               spaceFill(rng, board, x, y);
 }

void ensureConnectedness(JAVA& rng, Board& board, int x, int y)
 {
   bool fillFill = false;
   if (-1 == x)
    {
      fillFill = true;
      for (y = 0; y < MAX; ++y)
         for (x = 0; x < MAX; ++x)
            board.cell[y][x].v = false;

      x = 0;
      y = 0;
    }
   board.cell[y][x].v = true;

   std::stack<int> xs;
   std::stack<int> ys;

   xs.push(x);
   ys.push(y);

   while (false == xs.empty())
    {
      if ((x - 1 >  -1) && (false == board.cell[y][x - 1].v) && ((OPEN == board.cell[y][x].le) || (DOOR == board.cell[y][x].le)))
       {
         board.cell[y][x - 1].v = true;
         xs.push(x - 1);
         ys.push(y);
       }
      if ((x + 1 < MAX) && (false == board.cell[y][x + 1].v) && ((OPEN == board.cell[y][x].re) || (DOOR == board.cell[y][x].re)))
       {
         board.cell[y][x + 1].v = true;
         xs.push(x + 1);
         ys.push(y);
       }
      if ((y - 1 >  -1) && (false == board.cell[y - 1][x].v) && ((OPEN == board.cell[y][x].ue) || (DOOR == board.cell[y][x].ue)))
       {
         board.cell[y - 1][x].v = true;
         xs.push(x);
         ys.push(y - 1);
       }
      if ((y + 1 < MAX) && (false == board.cell[y + 1][x].v) && ((OPEN == board.cell[y][x].de) || (DOOR == board.cell[y][x].de)))
       {
         board.cell[y + 1][x].v = true;
         xs.push(x);
         ys.push(y + 1);
       }

      x = xs.top();
      y = ys.top();
      xs.pop();
      ys.pop();
    }

   if (true == fillFill)
      for (y = 0; y < MAX; ++y)
         for (x = 0; x < MAX; ++x)
            if (false == board.cell[y][x].v)
             {
               bool found = false;
               int c = 0;
               int d = Rand(rng, 4);

               while ((false == found) && (c < 4))
                {
                  switch (d)
                   {
                  case 0:
                     if ((x - 1 >  -1) && (true == board.cell[y][x - 1].v) && (WALL == board.cell[y][x].le))
                      {
                        EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                        board.cell[y][x].le = type;
                        board.cell[y][x - 1].re = type;
                        found = true;
                      }
                     break;
                  case 1:
                     if ((y - 1 >  -1) && (true == board.cell[y - 1][x].v) && (WALL == board.cell[y][x].ue))
                      {
                        EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                        board.cell[y][x].ue = type;
                        board.cell[y - 1][x].de = type;
                        found = true;
                      }
                     break;
                  case 2:
                     if ((x + 1 < MAX) && (true == board.cell[y][x + 1].v) && (WALL == board.cell[y][x].re))
                      {
                        EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                        board.cell[y][x].re = type;
                        board.cell[y][x + 1].le = type;
                        found = true;
                      }
                     break;
                  case 3:
                     if ((y + 1 < MAX) && (true == board.cell[y + 1][x].v) && (WALL == board.cell[y][x].de))
                      {
                        EDGE type = Rand(rng, 8) ? OPEN : DOOR;
                        board.cell[y][x].de = type;
                        board.cell[y + 1][x].ue = type;
                        found = true;
                      }
                     break;
                   }

                  ++c;
                  d = (d + 1) & 3;
                }

               if (true == found)
                  ensureConnectedness(rng, board, x, y);
             }
 }

void removeEverything(Board& board)
 {
   for (int y = 0; y < MAX; ++y)
      for (int x = 0; x < MAX; ++x)
       {
         if (WALL == board.cell[y][x].le) board.cell[y][x].le = OPEN;
         if (WALL == board.cell[y][x].re) board.cell[y][x].re = OPEN;
         if (WALL == board.cell[y][x].ue) board.cell[y][x].ue = OPEN;
         if (WALL == board.cell[y][x].de) board.cell[y][x].de = OPEN;
         board.cell[y][x].v = true;
       }
 }

void connectLeft(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 8) + 8;

   while (--n)
    {
      int y = Rand(rng, MAX);
      EDGE type = Rand(rng, 8) ? OPEN : DOOR;
      board.cell[y][0].le = type;
    }
 }

void connectRight(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 8) + 8;

   while (--n)
    {
      int y = Rand(rng, MAX);
      EDGE type = Rand(rng, 8) ? OPEN : DOOR;
      board.cell[y][TOP].re = type;
    }
 }

void connectUp(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 8) + 8;

   while (--n)
    {
      int x = Rand(rng, MAX);
      EDGE type = Rand(rng, 8) ? OPEN : DOOR;
      board.cell[0][x].ue = type;
    }
 }

void connectDown(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 8) + 8;

   while (--n)
    {
      int x = Rand(rng, MAX);
      EDGE type = Rand(rng, 8) ? OPEN : DOOR;
      board.cell[TOP][x].de = type;
    }
 }

// I wanted a bunch of one-way doors in the maze, but the implementation crashes.
void addMoreDoors(JAVA& rng, Board& board)
 {
   int n = Rand(rng, 2048) + 1024;

   while (--n)
    {
      bool found = false;
      while (false == found)
       {
         int x = Rand(rng, MAX - 2) + 1;
         int y = Rand(rng, MAX - 2) + 1;
         int d = Rand(rng, 4);
         switch (d)
          {
         case 0:
            if (WALL == board.cell[y][x].le)
             {
               //int type = Rand(rng, 16);
               board.cell[y][x].le = OPEN; //(type & 12) ? OPEN : (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y][x - 1].re = OPEN; //(type & 12) ? OPEN : (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
         case 1:
            if (WALL == board.cell[y][x].re)
             {
               //int type = Rand(rng, 16);
               board.cell[y][x].re = OPEN; //(type & 12) ? OPEN : (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y][x + 1].le = OPEN; //(type & 12) ? OPEN : (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
         case 2:
            if (WALL == board.cell[y][x].ue)
             {
               //int type = Rand(rng, 16);
               board.cell[y][x].ue = OPEN; //(type & 12) ? OPEN : (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y - 1][x].de = OPEN; //(type & 12) ? OPEN : (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
         case 3:
            if (WALL == board.cell[y][x].de)
             {
               //int type = Rand(rng, 16);
               board.cell[y][x].de = OPEN; //(type & 12) ? OPEN : (type & 1) ? DOOR_UP : DOOR_DOWN;
               board.cell[y + 1][x].ue = OPEN; //(type & 12) ? OPEN : (type & 2) ? DOOR_UP : DOOR_DOWN;
               found = true;
             }
            break;
          }
       }
    }
 }

void generate(Board& board)
 {
   JAVA left  (Mix((unsigned int)(board.x - 1), board.x, board.y, board.z));
   JAVA right (Mix(board.x, (unsigned int)(board.x + 1), board.y, board.z));
   JAVA up    (Mix(board.x, board.y, (unsigned int)(board.y + 1), board.z));
   JAVA down  (Mix(board.x, (unsigned int)(board.y - 1), board.y, board.z));
   JAVA door  (Mix(0, board.x, board.y, 1));
   JAVA rng   (Mix(board.l, board.x, board.y, board.z));

   addDoors(door, board);
   if (0 != board.z)
    {
      addOffices(rng, board);
      spaceFill(rng, board, -1, -1);
      ensureConnectedness(rng, board, -1, -1);
      connectLeft(left, board);
      connectRight(right, board);
      connectUp(up, board);
      connectDown(down, board);
      addMoreDoors(rng, board);
    }
   else
    {
      removeEverything(board);
    }
 }

#include <sstream>
#include "MakeMap.h"

#include "Zone.h"

void getStart(int seed, Zone& firstZone, Zone& playerStart)
 {
   JAVA rng (Mix(seed, 0, 1, 2));
   firstZone.x = (rng.getNext() >> 17) + 0x40000000;
   firstZone.y = (rng.getNext() >> 17) + 0x40000000;
   firstZone.z = (rng.getNext() >> 17) + 0x40000000;
   playerStart.x = 2 * Rand(rng, 256) + 1;
   playerStart.y = 2 * Rand(rng, 256) + 1;
 }

#include <memory>
std::string makeMap(unsigned int x, unsigned int y, unsigned int z)
 {
   std::unique_ptr<Board> board = std::make_unique<Board>();
   board->x = x;
   board->y = y;
   board->z = z;
   board->l = 0; // The only non-arbitrary one.

    // Generate maze
   generate(*board);

   return MakeStr(*board);
 }

/*
#include <iostream>
int main (void)
 {
    // Print Maze
   std::cout << makeMap(10, 11, 13) << std::endl;

   return 0;
 }
*/
