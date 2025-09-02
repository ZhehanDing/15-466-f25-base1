#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

enum ItemType {
	Shape,
	Color
};

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	
	void level_init();
	bool level_complete();

	int level = 1;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	//glm::vec2 player_at = glm::vec2(0.0f);
	
	float elapsed_sum = 0.0f;

	struct Item {
		ItemType type;
		uint8_t index;
		glm::vec2 pos = glm::vec2(128.0f, 256.0f);
	};

	struct Slot {
		uint8_t shape;
		uint8_t color = 0;
		glm::vec2 pos = glm::vec2(128.0f, 256.0f);
	} slots[6];
	
	//----- drawing handled by PPU466 -----
	PPU466 ppu;
};
