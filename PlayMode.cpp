/*Creator: Alex Ding
  Credit to ChatGPT
  Method referrenced: 
  		Jialan Dong' assignment 1
  		https://github.com/xinyis991105/15-466-f20-base1/blob/master/
*/

#include "PlayMode.hpp"
#include <algorithm>
#include "gl_errors.hpp"
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"
#include "data_path.hpp"
#include "Load.hpp"


//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <bitset>

struct StreamContainer {
	std::vector<PPU466::Palette> palettes;
};
glm::vec2 player_pos = glm::vec2(144.0f, 120.0f); // 

struct Ball {
    glm::vec2 pos;
    glm::vec2 size = glm::vec2(8.0f, 8.0f);
    int type = 11; // 11=blue, 12=red, 13=yellow
	float fall_speed = 0.0f;
};
std::deque<Ball> balls;
// Hold Ball Detect
Ball held_ball;          
bool has_ball = false;  
std::ifstream palette_stream;
std::ifstream tile_stream;

//
Load<void> ps(LoadTagDefault, []() {
    palette_stream.open(data_path("../assets/palettes.asset"));
    tile_stream.open(data_path("../assets/tiles.asset"));
    return;
});

PlayMode::PlayMode() {
    std::vector<PPU466::Palette> palettes;
    std::vector<PPU466::Tile> tiles;

    read_chunk(palette_stream, std::string("pale"), &palettes);
    read_chunk(tile_stream, std::string("tile"), &tiles);
    palette_stream.close();
    tile_stream.close();

    std::cout << "Tiles loaded = " << tiles.size() 
              << ", Palettes loaded = " << palettes.size() << std::endl;

    // 
    for (size_t i = 0; i < tiles.size() && i < 256; i++) {
        ppu.tile_table[i] = tiles[i];
    }
    for (size_t i = 0; i < palettes.size() && i < 8; i++) {
        ppu.palette_table[i] = palettes[i];
    }

    // 
    ppu.background_color = glm::u8vec4(100, 100, 200, 255);


	ppu.tile_table[32].bit0 = {
		0b00000000,
		0b00010000,
		0b00010000,
		0b00010000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
	};
	ppu.tile_table[32].bit1 = {
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
	};
	
    ppu.palette_table[0] = {
		glm::u8vec4(0, 0, 0, 0),  // transparent
		glm::u8vec4(0, 0, 0, 0),    // black
		glm::u8vec4(0, 0, 0, 0),    // transparent
		glm::u8vec4(0, 0, 0, 0)     // transparent
	};;
	
	
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = 0;
		}
	}

    // 

}

PlayMode::~PlayMode() { }

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    // 
	if (evt.type == SDL_EVENT_KEY_DOWN) {
		if (evt.key.key == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.type == SDL_EVENT_KEY_DOWN) {
			if (evt.key.key == SDLK_SPACE) {
				if (has_ball) {
					held_ball.pos = player_pos + glm::vec2(0, 8.0f); 
					held_ball.fall_speed = -120.0f; 
					balls.push_back(held_ball);
					has_ball = false;
				}
				return true;
			}
}

	}
	else if (evt.type == SDL_EVENT_KEY_UP) {
		if (evt.key.key == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.key == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}
		
    return false;
}


void PlayMode::update(float elapsed) {

	// Spaceship speed
    constexpr float PlayerSpeed = 80.0f; 

    if (left.pressed)  player_pos.x -= PlayerSpeed * elapsed;
    if (right.pressed) player_pos.x += PlayerSpeed * elapsed;
    if (down.pressed)  player_pos.y -= PlayerSpeed * elapsed;
    if (up.pressed)    player_pos.y += PlayerSpeed * elapsed;

    // board limitation
    if (player_pos.x < 16) player_pos.x = 16;
    if (player_pos.x > 240) player_pos.x = 240;
    if (player_pos.y < 16) player_pos.y = 16;
    if (player_pos.y > 224) player_pos.y = 224;

	//GPT Debug
	for (auto it = balls.begin(); it != balls.end();) {
    it->pos.y -= it->fall_speed * elapsed;

    // Hit Check
    if (!has_ball &&
        fabs(it->pos.x - player_pos.x) < 8.0f &&  
        fabs(it->pos.y - player_pos.y) < 8.0f) {  

        held_ball = *it;     // catch ball
        has_ball = true;     
        it = balls.erase(it); 
        continue;
    }

    // Out Check
    if (it->pos.y < 0.0f || it->pos.y > 240.0f) {
        it = balls.erase(it);
    } else {
        ++it;
    	}
	}
	// Ball Generation
	static float spawn_timer = 0.0f;
    spawn_timer += elapsed;
	if (spawn_timer > 1.0f) {
    	spawn_timer = 0.0f;
		Ball b;
		b.type = 11+ rand() % 3; 
		int rand_x = 16 + (rand() % (240 - 16 + 1));
		b.pos = glm::vec2((float)rand_x, 200.0f);
		b.fall_speed=30.0f+(rand() % 30);
		balls.push_back(b);
	}
	// Ball Falling Speed
	for (auto &b : balls) {
        b.pos.y -= b.fall_speed * elapsed;
    }
	// Remove Ball (GPT Fixed)
	while (!balls.empty() && balls.front().pos.y < 0 ) {
    balls.pop_front();
	}

    // Eeset downs
    left.downs = right.downs = up.downs = down.downs = 0;
}


void PlayMode::draw(glm::uvec2 const &drawable_size) {

    float x = player_pos.x;
    float y = player_pos.y;
	//Space ship drawing
    ppu.sprites[0].x = uint8_t(x - 16);
    ppu.sprites[0].y = uint8_t(y);
    ppu.sprites[0].index = 0;
    ppu.sprites[0].attributes = 0;

    ppu.sprites[1].x = uint8_t(x - 8);
    ppu.sprites[1].y = uint8_t(y);
    ppu.sprites[1].index = 1;
    ppu.sprites[1].attributes = 1;

    ppu.sprites[2].x = uint8_t(x);
    ppu.sprites[2].y = uint8_t(y);
	// Holding Ball
	if (has_ball) {
    if (held_ball.type == 11) {
        ppu.sprites[2].index = 11;
        ppu.sprites[2].attributes = 1;
    } else if (held_ball.type == 12) {
        ppu.sprites[2].index = 12;
        ppu.sprites[2].attributes = 2;
    } else if (held_ball.type == 13) {
        ppu.sprites[2].index = 13;
        ppu.sprites[2].attributes = 3;
    }
} 
	else {
    ppu.sprites[2].index = 2;  
    ppu.sprites[2].attributes = 2;
	}
    ppu.sprites[2].index = 2;
    ppu.sprites[2].attributes = 1;

    ppu.sprites[3].x = uint8_t(x + 8);
    ppu.sprites[3].y = uint8_t(y);
    ppu.sprites[3].index = 3;
    ppu.sprites[3].attributes = 3;

    ppu.sprites[4].x = uint8_t(x - 8);
    ppu.sprites[4].y = uint8_t(y + 8);
    ppu.sprites[4].index = 4;
    ppu.sprites[4].attributes = 4;

    ppu.sprites[5].x = uint8_t(x);
    ppu.sprites[5].y = uint8_t(y + 8);
    ppu.sprites[5].index = 5;
    ppu.sprites[5].attributes = 5;

    ppu.sprites[6].x = uint8_t(x + 8);
    ppu.sprites[6].y = uint8_t(y + 8);
    ppu.sprites[6].index = 6;
    ppu.sprites[6].attributes = 6;

    ppu.sprites[7].x = uint8_t(x - 8);
    ppu.sprites[7].y = uint8_t(y + 16);
    ppu.sprites[7].index = 7;
    ppu.sprites[7].attributes = 7;

    ppu.sprites[8].x = uint8_t(x);
    ppu.sprites[8].y = uint8_t(y + 16);
    ppu.sprites[8].index = 8;
    ppu.sprites[8].attributes = 8;

    ppu.sprites[9].x = uint8_t(x + 8);
    ppu.sprites[9].y = uint8_t(y + 16);
    ppu.sprites[9].index = 9;
    ppu.sprites[9].attributes =7;

	// Ball Drawing (Still bug)
	int sprite_id = 10; 
	for (auto &b : balls) {
		if (sprite_id >= 64) break; 
		ppu.sprites[sprite_id].x = uint8_t(b.pos.x);
		ppu.sprites[sprite_id].y = uint8_t(b.pos.y);
		if (b.type == 11) { // blue
			ppu.sprites[sprite_id].index = 11;
			ppu.sprites[sprite_id].attributes = 1;
		} else if (b.type == 12) { // red
			ppu.sprites[sprite_id].index = 12;
			ppu.sprites[sprite_id].attributes = 2;
		} else if (b.type == 13) { // yellow
			ppu.sprites[sprite_id].index = 13;
			ppu.sprites[sprite_id].attributes = 3;
		}
		sprite_id++;
	}
	std::cout << "Balls count: " << balls.size() << std::endl;
		for (auto &b : balls) {
    	std::cout << "Ball at (" << b.pos.x << ", " << b.pos.y << ") type " << b.type << std::endl;
}

    ppu.draw(drawable_size);
}
