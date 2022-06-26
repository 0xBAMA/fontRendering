#include <string>
#include <vector>
#include <iostream>

// image input
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// image output
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main(int argc, char const *argv[]) {
	int w,h,comp;
	unsigned char * data = stbi_load( std::string("colorClearBackground.png").c_str(), &w, &h, &comp, STBI_rgb_alpha );

	std::vector< unsigned char > output;
	output.resize( 128 * 256 * 4 * 2);


	std::cout << " started writing data" << std::endl << std::flush;

	// per character
	for ( int i = 0; i < 256; i++ ) {
		// per pixel
		int sourceBaseY = i * 16;
		for ( int x = 0; x < 8; x++ ) {
			for( int y = 0; y < 16; y++ ) {
				// write to correct location
				int baseX = ( i % 16 ) * 8;
				int baseY = ( i / 16 ) * 16;

				int rIndex = ( x + ( ( y + sourceBaseY ) * 8 ) ) * 4;
				int wIndex = ( baseX + x + ( baseY + y ) * 128 ) * 4;
				output[ wIndex ] = data[ rIndex ];
				output[ wIndex+1 ] = data[ rIndex+1 ];
				output[ wIndex+2 ] = data[ rIndex+2 ];
				output[ wIndex+3 ] = 255;
			}
		}
	}

	std::cout << " finished writing data" << std::endl << std::flush;

	stbi_write_png( std::string( "out.png" ).c_str(), 128, 256, 4, &output[ 0 ], 128 * 4 );

	return 0;
}
