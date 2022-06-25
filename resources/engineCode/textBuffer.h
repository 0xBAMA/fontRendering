#include "includes.h"
#ifndef TEXTBUFFER
#define TEXTBUFFER

struct coloredChar {
	unsigned char data[ 4 ] = { 255, 255, 255, 0 };
};

class textBuffer {
public:
	textBuffer( unsigned int x, unsigned int y )
		: dimensions( x, y )
		, bufferSize( 64, 64 )
		, offset( 0, 0 )
		{
		buffer.resize( bufferSize.x * bufferSize.y );
	}

	// size of the buffer
	glm::uvec2 dimensions;	// size of display
	glm::uvec2 bufferSize;	// size of the buffer
	glm::ivec2 offset;			// offset of the buffer within the display
	void Update() {
		// check for buffer resize
		static glm::uvec2 lastUpdateBufferSize;
		if ( bufferSize != lastUpdateBufferSize ) {
			lastUpdateBufferSize = bufferSize;
			// resize the buffer and resend the data
			ResetBuffer();
			Repopulate();
			ResendData();
		}
	}

	void ResetBuffer() {
		buffer.resize( 0 );
		coloredChar temp;
		buffer.resize( bufferSize.x * bufferSize.y, temp );
	}

	// go from string to the buffer
	void Repopulate() {
		// std::random_device r;
		// std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
		// auto gen = std::mt19937_64( s );
		// std::uniform_int_distribution<uint8_t> dist( 0, 255 );
		std::string message(  "\"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\"" );
		const unsigned int maxSize = bufferSize.x * bufferSize.y;
		for ( unsigned int i = 0; i < message.length() && i < maxSize; i++ ) {
			// buffer[ i ].data[ 0 ] = dist( gen );
			// buffer[ i ].data[ 1 ] = dist( gen );
			// buffer[ i ].data[ 2 ] = dist( gen );
			buffer[ i ].data[ 3 ] = message.c_str()[ i ];
			// buffer[ i ].data[ 3 ] = dist( gen );
		}
	}

	// send the data to the GPU
	GLint dataTexture;	// texture handle for the GPU
	void ResendData() {
		std::vector< uint8_t > imageData;
		imageData.reserve( bufferSize.x * bufferSize.y * 4 );
		for ( unsigned int y = 0; y < bufferSize.y; y++ )
		for ( unsigned int x = 0; x < bufferSize.x; x++ )
		for ( unsigned int c = 0; c < 4; c++ ) {
			unsigned int index = x + y * bufferSize.x;
			imageData.push_back( ( index < buffer.size() ) ? buffer[ index ].data[ c ] : 0 );
		}
		glBindTexture( GL_TEXTURE_2D, dataTexture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, bufferSize.x, bufferSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &imageData[ 0 ] );
	}


private:
	std::vector< coloredChar > buffer;
};

#endif
