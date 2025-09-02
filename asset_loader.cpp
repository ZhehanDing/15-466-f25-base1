// Authorized by: Jialan Dong
// Cursor AI assistant used
/*
Method referenced: 
    https://github.com/bobowitz/15666-game1/blob/master/
    https://github.com/xinyis991105/15-466-f20-base1/blob/master/
    https://github.com/Chipxiang/Jump-Guy/blob/master/

*/ 

#include "Load.hpp"
#include "load_save_png.hpp"
#include "read_write_chunk.hpp"
#include "PPU466.hpp"
#include "data_path.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>

void load_assets() {
    std::vector<PPU466::Tile> tiles;
    std::vector<PPU466::Palette> palettes;

    std::ifstream fileList;
    // std::cout << "exe_path = " << data_path("./") << "\n";
    fileList.open(data_path("../assets/asset_list.txt")); // open asset list file
    if (!fileList.is_open()) { // fail to open asset list file
        std::cerr << "Error: Failed to open asset list file" << std::endl;
        return;
    }
    std::string line;
    bool paletteNumWarned = false;
    while (std::getline(fileList, line)) {

    // Build file path and load PNG
    std::string filePath = data_path("../assets/" + line + ".png");
    std::cout << "Processing PNG file: " << filePath << std::endl;

    glm::uvec2 size;
    std::vector<glm::u8vec4> data;
    load_png(filePath, &size, &data, LowerLeftOrigin);

    // Divide the image into tiles (8x8)
    int w = size.x / 8;
    int h = size.y / 8;

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) { // iterate over each tile

            // Initialize empty tile and palette
            PPU466::Tile tile = { {0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0} };
            PPU466::Palette palette;
            int colorCount = 0;

            // Iterate over every pixel inside the 8x8 tile
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    int index = (j * 8 + y) * w * 8 + i * 8 + x;
                    glm::u8vec4 color = data[index];

                    // Check if the color already exists in the palette
                    int colorIndex = -1;
                    for (int c = 0; c < colorCount; c++) {
                        if (color == palette[c]) {
                            colorIndex = c;
                            break;
                        }
                    }

                    // If not found, add the new color to the palette
                    if (colorIndex == -1) {
                        palette[colorCount] = color;
                        colorIndex = colorCount;
                        colorCount++;
                    }

                    // Store the color index in the tile bitplanes
                    tile.bit0[y] |= (colorIndex & 1) << x;
                    tile.bit1[y] |= ((colorIndex >> 1) & 1) << x;
                }
            }

            // Save the generated tile
            tiles.push_back(tile);

            // Save palette (only keep up to 8 palettes)
            if (palettes.size() < 8) {
                palettes.push_back(palette);
            } else if (!paletteNumWarned) {
                std::cout << "Palette exceed limitation! New Palette dumped!" << std::endl;
                paletteNumWarned = true;
            }
        }
    }

    std::cout << "Ended with tile count = " << tiles.size() << std::endl;
}

    fileList.close();
    std::ofstream palettes_stream;
	std::ofstream tiles_stream;
	palettes_stream.open(data_path("../assets/palettes.asset"));
	tiles_stream.open(data_path("../assets/tiles.asset"));
	write_chunk(std::string("pale"), palettes, &palettes_stream);
	write_chunk(std::string("tile"), tiles, &tiles_stream);
	palettes_stream.close();
	tiles_stream.close();
    
    std::cout << "Assets Loading Done. " << tiles.size() << " Tiles And " 
              << palettes.size() << " Palettes Loaded" << std::endl;
}

Load<void> asset_loader(LoadTagEarly, load_assets);