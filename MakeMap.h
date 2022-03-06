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

const char * selectWallMap(EDGE right, EDGE left)
 {
   switch (right)
    {
   case WALL:
      return "#";
   case OPEN:
      return " ";
   case DOOR: // Getting rid of door concept, because I don't want to program the semi-wall.
      return " ";
   case DOOR_UP:
      if (DOOR_UP == left)
         return "U";
      else if (DOOR_DOWN == left)
         return "u";
      else
         return "!";
   case DOOR_DOWN:
      if (DOOR_UP == left)
         return "d";
      else if (DOOR_DOWN == left)
         return "D";
      else
         return "!";
   case CLOSED_DOOR:
      return "!";
   case LOCKED_DOOR:
      return "!";
   case STAIRS:
      return "!";
    }
   return "!";
 }

std::string MakeStr(const Board& board)
 {
   std::string str;
   for (int y = TOP; y > -1; --y)
    {
      for (int x = 0; x < MAX; ++x)
       {
         if ((0 == x) && (TOP == y)) str.append("#").append(selectWallMap(board.cell[y][x].de, board.cell[y][x].de)).append("#");
/**/     else if ((TOP == x) && (TOP == y)) str.append(selectWallMap(board.cell[y][x].de, board.cell[y][x].de));
         else if (TOP == y) str.append(selectWallMap(board.cell[y][x].de, board.cell[y][x].de)).append("#");
         else if (0 == x) str.append("#").append(selectWallMap(board.cell[y][x].de, board.cell[y + 1][x].ue))
           .append((((OPEN == board.cell[y][x].de) && (OPEN == board.cell[y + 1][x].re) && (OPEN == board.cell[y][x].re)) ? " " : "#"));
         else if (TOP == x) str.append(selectWallMap(board.cell[y][x].de, board.cell[y + 1][x].ue)); //.append("#");
         else str.append(selectWallMap(board.cell[y][x].de, board.cell[y + 1][x].ue))
           .append((((OPEN == board.cell[y][x + 1].de) && (OPEN == board.cell[y][x].de) && (OPEN == board.cell[y + 1][x].re) && (OPEN == board.cell[y][x].re)) ? " " : "#"));
       }
      //str.append("\n");
      for (int x = 0; x < MAX; ++x)
       {
         if (0 == x) str.append(selectWallMap(board.cell[y][x].le, board.cell[y][x].le));
         if (true == board.cell[y][x].v) str.append(" ");
         else str.append("!");
         if (TOP == x) ; //str.append(selectWallMap(board.cell[y][x].re, board.cell[y][x].re));
         else str.append(selectWallMap(board.cell[y][x].re, board.cell[y][x + 1].le));
       }
      //str.append("\n");
//      if (0 == y)
//       {
//         for (int x = 0; x < MAX; ++x)
//          {
//            if (0 == x) str.append("#");
//            str.append(selectWallMap(board.cell[y][x].ue, board.cell[y][x].ue)).append("#");
//          }
//         //str.append("\n");
//       }
    }
   return str;
 }
