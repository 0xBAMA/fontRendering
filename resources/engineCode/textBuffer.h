#include "includes.h"
#ifndef TEXTBUFFER
#define TEXTBUFFER

struct coloredChar {
	unsigned char data[ 4 ];
};

class textBuffer {
public:
	textBuffer( int x, int y ) : dimensions( x, y ) {
		buffer.resize( x * y );
	}

	// size of the buffer
	glm::ivec2 dimensions;
	void Update() {
		// check for buffer resize
		static glm::ivec2 lastUpdateDimensions;
		if ( dimensions != lastUpdateDimensions ) {
			lastUpdateDimensions = dimensions;
			// resize the buffer and resend the data
			Repopulate();
			ResendData();
		}
	}

	// go from string to the buffer
	void Repopulate() {
		std::string message( "This is a string which is being displayed. It will be displayed as a string of white characters. Thanks for looking at the displayed text. Bye." );
		for ( unsigned int i = 0; i < message.length() && i < buffer.size(); i++ ) {
			buffer[ i ].data[ 3 ] = message.c_str()[ i ];
		}
	}

	// send the data to the GPU
	GLint dataTexture;	// texture handle for the GPU
	void ResendData() {
		std::vector< uint8_t > imageData;
		imageData.reserve( dimensions.x * dimensions.y * 4 );
		for ( int y = 0; y < dimensions.y; y++ )
		for ( int x = 0; x < dimensions.x; x++ )
		for ( int c = 0; c < 4; c++ ) {
			imageData.push_back( buffer[ x + y * dimensions.x ].data[ c ] );
		}
		glBindTexture( GL_TEXTURE_2D, dataTexture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &imageData[ 0 ] );
	}


private:
	std::vector< coloredChar > buffer;
};

#endif
