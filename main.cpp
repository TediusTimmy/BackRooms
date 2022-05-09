/*
   Escape the BackRooms.
   A winding, non-Euclidean maze game and walking simulator.

   Authors:
      David Barr (Original Engine)
      Thomas DiModica (Additional Engine Programming)
*/

/*
	Simple example of RayCastWorld Pixel Game Engine Extension
	"My Kneeeeeeeees!!!!" - javidx9

	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2020 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Relevant Video: https://youtu.be/Vij_obgv9h4

	Links
	~~~~~
	YouTube:	https://www.youtube.com/javidx9
				https://www.youtube.com/javidx9extra
	Discord:	https://discord.gg/WhwHUMV
	Twitter:	https://www.twitter.com/javidx9
	Twitch:		https://www.twitch.tv/javidx9
	GitHub:		https://www.github.com/onelonecoder
	Patreon:	https://www.patreon.com/javidx9
	Homepage:	https://www.onelonecoder.com

	Community Blog: https://community.onelonecoder.com

	Author
	~~~~~~
	David Barr, aka javidx9, ©OneLoneCoder 2018, 2019, 2020
*/

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define OLC_PGEX_RAYCASTWORLD
#include "olcPGEX_RayCastWorld.h"

#include "Zone.h"
void getStart(int, Zone&, Zone&);
std::string makeMap(unsigned int x, unsigned int y, unsigned int z);
const float scale = 3.0f;

class Ayers
 {
public:
   std::string m;
   uint64_t a;
 };

const std::size_t cacheMax = 100U;
std::map<Zone, Ayers> cache;
const std::string& getCache(unsigned int x, unsigned int y, unsigned int z)
 {
   static uint64_t h = 0U;
   ++h;
   Zone zone;
   zone.x = x;
   zone.y = y;
   zone.z = z;

   if (cache.end() == cache.find(zone))
    {
      cache[zone].m = makeMap(x, y, z);
    }
   cache[zone].a = h;

   if (cache.size() > cacheMax)
    {
      std::map<Zone, Ayers>::iterator found = cache.begin();
      uint64_t b = found->second.a;
      for (std::map<Zone, Ayers>::iterator iter = cache.begin(); cache.end() != iter; ++iter)
       {
         if (iter->second.a < b)
          {
            b = iter->second.a;
            found = iter;
          }
       }
      cache.erase(found);
    }

   return cache[zone].m;
 }

// ADAPTOR CLASS - Override the RayCastWorld Engine and fill in the blanks!
class ExampleGame : public olc::rcw::Engine
{
public:
	ExampleGame(const int screen_w, const int screen_h, const float fov)
		: olc::rcw::Engine(screen_w, screen_h, fov)
	{ 
      getStart(std::time(nullptr), zone, player);

		vWorldSize = { 512, 512 };
	}

   void ImTheMap(const olc::vf2d& player)
    {
      int tx = int(std::floor(player.x / scale));
      int ty = int(std::floor(player.y / scale));
      for (int y = ty - 32; y < ty + 32; ++y)
         for (int x = tx - 32; x < tx + 32; ++x)
          {
            if ((x == tx) && (y == ty))
               pge->Draw(vScreenSize.x - 64 + x - tx + 32, y - ty + 32, olc::Pixel(0, 0, 255));
            else
             {
               char tile = getMapTile(x, y, zone.z);
               if ('#' == tile)
                  pge->Draw(vScreenSize.x - 64 + x - tx + 32, y - ty + 32, olc::Pixel(0, 0, 0));
               else if (' ' == tile)
                  pge->Draw(vScreenSize.x - 64 + x - tx + 32, y - ty + 32, olc::Pixel(255, 255, 255));
               else
                  pge->Draw(vScreenSize.x - 64 + x - tx + 32, y - ty + 32, olc::Pixel(0, 255, 0));
             }
          }
    }

   void correctPlayer(olc::vf2d& player, const Path& path)
    {
      if (player.x > vWorldSize.x * scale)
       {
         player.x -= vWorldSize.x * scale;
         ++zone.x;
       }
      else if (player.x < 0)
       {
         player.x += vWorldSize.x * scale;
         --zone.x;
       }
      else if (player.y > vWorldSize.y * scale)
       {
         player.y -= vWorldSize.y * scale;
         ++zone.y;
       }
      else if (player.y < 0)
       {
         player.y += vWorldSize.y * scale;
         --zone.y;
       }

      if (path.z != zone.z)
       {
         zone.z = path.z;
       }
    }

   const Zone& getPlayer()
    {
      return player;
    }

   const Zone& getZone()
    {
      return zone;
    }

protected:
	// User implementation to retrieve appropriate graphics for scenery
	olc::Pixel SelectSceneryPixel(const int /*tile_x*/, const int /*tile_y*/, const olc::rcw::Engine::CellSide side, const float sample_x, const float sample_y, const float distance) override
	{
		olc::Pixel p;

		// Choose appropriate colour
		switch (side)
		{
			case olc::rcw::Engine::CellSide::Top: // Location is "Sky"
				p = olc::GREY;
            if (sample_x < 0.05f || sample_x > 0.95f || sample_y < 0.05f || sample_y > 0.95f)
					p = olc::WHITE;
				break;

			case olc::rcw::Engine::CellSide::Bottom: // Location is "Ground"
				p = olc::Pixel(100, 50, 0);
				break;

			default: // Location is "Wall"
            if (sample_x < 0.025f || sample_x > 0.975f)
               p = olc::Pixel(255, 100, 0);
            else
               p = olc::Pixel(200, 255, 0);
				break;
		}

		// Apply directional lighting, by first creating a shadow scalar...
		//float fShadow = 1.0f;
		//switch (side)
		//{
		//	case olc::rcw::Engine::CellSide::South: fShadow = 0.3f; break;
		//	case olc::rcw::Engine::CellSide::East:  fShadow = 0.3f; break;
		//}

		// ...also shade by distance...
		float fDistance = 1.0f - std::min(distance / 256.0f, 1.0f);

		// ...and applying it to sampled pixel
		p.c.r = uint8_t(float(p.c.r) * fDistance);
		p.c.g = uint8_t(float(p.c.g) * fDistance);
		p.c.b = uint8_t(float(p.c.b) * fDistance);

		return p;
	}

   char getMapTile(const int tx, const int ty, const unsigned int z)
    {
		if (tx < 0)
         return getCache(zone.x - 1, zone.y, z)[ty * vWorldSize.x + tx + vWorldSize.x];
		else if (tx >= vWorldSize.x)
         return getCache(zone.x + 1, zone.y, z)[ty * vWorldSize.x + tx - vWorldSize.x];
		else if (ty < 0)
         return getCache(zone.x, zone.y - 1, z)[(ty + vWorldSize.y) * vWorldSize.x + tx];
		else if (ty >= vWorldSize.y)
         return getCache(zone.x, zone.y + 1, z)[(ty - vWorldSize.y) * vWorldSize.x + tx];
      else
			return getCache(zone.x, zone.y, z)[ty * vWorldSize.x + tx];
    }

	// User implementation to retrieve if a particular tile is solid
	bool IsLocationSolid(const float tile_x, const float tile_y, const Path& z) override
	{
      int tx = (int)(std::floor(tile_x / scale));
      int ty = (int)(std::floor(tile_y / scale));
      return getMapTile(tx, ty, z.z) == '#';
	}

   Path GetBasePath() override
    {
      return Path(zone.z);
    }

   bool IsLocationSpecial(const float tile_x, const float tile_y, const Path& z)
    {
      int tx = (int)(std::floor(tile_x / scale));
      int ty = (int)(std::floor(tile_y / scale));
      char loc = getMapTile(tx, ty, z.z) | 32;
      return (loc == 'd') | (loc == 'u');
    }

   void HandleSpecial(const float tile_x, const float tile_y, const olc::rcw::Engine::CellSide side, Path& z)
    {
      int tx = (int)(std::floor(tile_x / scale));
      int ty = (int)(std::floor(tile_y / scale));

      if (true == z.valid) // Have I already processed this tile?
       {
         if ((z.mx == tx) && (z.my == ty))
            return;
       }
      z.valid = true;
      z.mx = tx;
      z.my = ty;

      char loc = getMapTile(tx, ty, z.z);
      switch (loc)
       {
      case 'u':
         if ((side == olc::rcw::Engine::CellSide::North) || (side == olc::rcw::Engine::CellSide::East))
            --z.z;
         else
            ++z.z;
         break;
      case 'U':
         ++z.z;
         break;
      case 'd':
         if ((side == olc::rcw::Engine::CellSide::North) || (side == olc::rcw::Engine::CellSide::East))
            ++z.z;
         else
            --z.z;
         break;
      case 'D':
         --z.z;
         break;
       }
    }

	// NOTE! Objects are not used in this demonstration ===================

	// User implementation to retrieve dimensions of an in game object
	float GetObjectWidth(const uint32_t /*id*/) override
	{ return 1; }

	float GetObjectHeight(const uint32_t /*id*/) override
	{ return 1; }

	// User implementation to retrieve appropriate graphics for objects
	olc::Pixel SelectObjectPixel(const uint32_t /*id*/, const float /*sample_x*/, const float /*sample_y*/, const float /*distance*/, const float /*angle*/) override
	{ return olc::BLACK; }

private:
   Zone zone, player;
	olc::vi2d vWorldSize;
};



class RayCastWorldDemo_SIMPLE : public olc::PixelGameEngine
{
public:
	RayCastWorldDemo_SIMPLE()
	{
		sAppName = "RayCastWorld - BackRooms Alpha 2";
	}

public:
	bool OnUserCreate() override
	{
		// Create game object
		pGame.reset(new ExampleGame(ScreenWidth(), ScreenHeight(), 3.14159f / 3.333f));

      getCache(pGame->getZone().x, pGame->getZone().y, pGame->getZone().z);

		// Add an object "player"
		std::shared_ptr<olc::rcw::Object> player = std::make_shared<olc::rcw::Object>();
		player->pos = { (pGame->getPlayer().x + .5f) * scale, (pGame->getPlayer().y + .5f) * scale };
		player->bVisible = false;
      player->path = Path(pGame->getZone().z);

		// Insert into game world
		pGame->mapObjects.insert_or_assign(0, player);		
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Handle User Input =================================================
		auto& player = pGame->mapObjects[0];

		if (GetKey(olc::Key::A).bHeld || (GetKey(olc::Key::LEFT).bHeld && !GetKey(olc::Key::SPACE).bHeld)) // Turn Left
			player->Turn(-fPlayerMoveSpeed * 0.1f * fElapsedTime);
		
		if (GetKey(olc::Key::D).bHeld || (GetKey(olc::Key::RIGHT).bHeld && !GetKey(olc::Key::SPACE).bHeld)) // Turn Right
			player->Turn(+fPlayerMoveSpeed * 0.1f * fElapsedTime);

		// Reset speed and velocity so player doesnt move if keys are not pressed
		player->Stop();

		// Walk Forward
		if (GetKey(olc::Key::W).bHeld || GetKey(olc::Key::UP).bHeld) player->Walk(+fPlayerMoveSpeed);
		// Walk Backwards
		if (GetKey(olc::Key::S).bHeld || GetKey(olc::Key::DOWN).bHeld) player->Walk(-fPlayerMoveSpeed);
		// Strafe Right
		if (GetKey(olc::Key::E).bHeld || (GetKey(olc::Key::RIGHT).bHeld && GetKey(olc::Key::SPACE).bHeld)) player->Strafe(+fPlayerMoveSpeed);
		// Strafe Left
		if (GetKey(olc::Key::Q).bHeld || (GetKey(olc::Key::LEFT).bHeld && GetKey(olc::Key::SPACE).bHeld)) player->Strafe(-fPlayerMoveSpeed);

		if (GetKey(olc::Key::I).bHeld && GetKey(olc::Key::L).bHeld && GetKey(olc::Key::M).bHeld) player->path = Path(0);

		// Update & Render World ==================================================

		// Update ray cast world engine - this will move the player object
		pGame->Update(fElapsedTime);

      // If the player has transitioned zones, fix coordinates.
      pGame->correctPlayer(player->pos, player->path);

		// Link the camera to the play position
		pGame->SetCamera(player->pos, player->fHeading);

		// Render the scene!
		pGame->Render();

      // Where the bloody hell AM I?
      pGame->ImTheMap(player->pos);

		// Draw the players position, cos why not?
		DrawString({ 10,10 }, player->pos.str() + " " + std::to_string(pGame->getZone().z), olc::BLACK);
		return true;
	}

private:
	float fPlayerMoveSpeed = 16.0f;
	std::unique_ptr<ExampleGame> pGame = nullptr;
};

int main()
{
	RayCastWorldDemo_SIMPLE demo;
	if (demo.Construct(640, 480, 2, 2))
		demo.Start();
	return 0;
}