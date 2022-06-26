#include "includes.h"
#ifndef TEXTBUFFER
#define TEXTBUFFER

#include "extendedASCIIdefines.h"

struct coloredChar {
	unsigned char data[ 4 ] = { 255, 255, 255, 0 };

	coloredChar() {}
	coloredChar( unsigned char c ) {
		data[ 3 ] = c;
	}

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
	bool updateFlag = false;
	void Update() {
		// check for buffer resize
		static glm::uvec2 lastUpdateBufferSize;
		if ( bufferSize != lastUpdateBufferSize || updateFlag ) {
			lastUpdateBufferSize = bufferSize;
			// resize the buffer and resend the data
			ResetBuffer();
			fillWithTestString();
			ResendData();
		}
	}

	void ResetBuffer() {
		buffer.resize( 0 );
		coloredChar temp;
		buffer.resize( bufferSize.x * bufferSize.y, temp );
	}

	// go from string to the buffer
	void fillWithTestString() {
		// std::random_device r;
		// std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
		// auto gen = std::mt19937_64( s );
		// std::uniform_int_distribution<uint8_t> dist( 0, 255 );
		// std::string message( "\"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\"" );
		// const unsigned int maxSize = bufferSize.x * bufferSize.y;
		// for ( unsigned int i = 0; i < message.length() && i < maxSize; i++ ) {
		// 	// buffer[ i ].data[ 0 ] = sin( color ) * 127 + 128;
		// 	// buffer[ i ].data[ 1 ] = cos( color ) * 127 + 128;
		// 	// buffer[ i ].data[ 2 ] = sin( color ) * 127 + 128;
		// 	buffer[ i ].data[ 3 ] = message.c_str()[ i ];
		// 	// buffer[ i ].data[ 3 ] = dist( gen );
		// }

		std::vector< std::vector< coloredChar > > lineBuffer;
		lineBuffer.resize( bufferSize.y );
		for ( unsigned int y = 0; y < bufferSize.y; y++ ) {
			lineBuffer[ y ].resize( bufferSize.x );
			for ( unsigned int x = 0; x < bufferSize.x; x++ ) {
				lineBuffer[ y ][ x ] = coloredChar( 0 );
			}
		}

		std::string testString( "mat3 rotZ ( float t ) {\n\rfloat s = sin( t );\n\rfloat c = cos( t );\n\rreturn mat3( c, s, 0., -s, c, 0., 0., 0., 1. );\n}\n\nmat3 rotX ( float t ) {\n\rfloat s = sin( t );\n\rfloat c = cos( t );\n\rreturn mat3( 1., 0., 0., 0., c, s, 0., -s, c );\n}\n\nmat3 rotY ( float t ) {\n\rfloat s = sin( t );\n\rfloat c = cos( t );\n\rreturn mat3 (c, 0., -s, 0., 1., 0, s, 0, c);\n}\n\nfloat de ( vec3 p ){\n\rvec2 rm = radians( 360.0 ) * vec2( 0.468359, 0.95317 ); // vary x,y 0.0 - 1.0\n\rmat3 scene_mtx = rotX( rm.x ) * rotY( rm.x ) * rotZ( rm.x ) * rotX( rm.y );\n\rfloat scaleAccum = 1.;\n\rfor( int i = 0; i < 18; ++i ) {\n\r\rp.yz = sqrt( p.yz * p.yz + 0.16406 );\n\r\rp *= 1.21;\n\r\rscaleAccum *= 1.21;\n\r\rp -= vec3( 2.43307, 5.28488, 0.9685 );\n\r\rp = scene_mtx * p;\n\r}\n\rreturn length( p ) / scaleAccum - 0.15;\n}\n" );

		glm::uvec2 cursorPosition = glm::uvec2( 0 );
		for ( unsigned char c : testString ){
			// process the character
			if( c == '\n' ){
				cursorPosition.x = 0;
				cursorPosition.y++;
			} else if ( c == '\r' ) {
				cursorPosition.x += 2;
			} else {
				lineBuffer[ cursorPosition.y ][ cursorPosition.x ] = coloredChar( c );
				cursorPosition.x++;
			}

			// bounds check
			if ( cursorPosition.x >= bufferSize.x ) {
				// same as a newline
				cursorPosition.x = 0;
				cursorPosition.y++;
				if ( cursorPosition.y >= bufferSize.y ) {
					break; // buffer can't contain any more
				}
			}
		}

		// put it into the the 1d representation, buffer
		for ( unsigned int y = 0; y < bufferSize.y; y++ )
		for ( unsigned int x = 0; x < bufferSize.x; x++ ) {
			buffer[ x + y * bufferSize.x ] = lineBuffer[ y ][ x ];
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
	// I don't like doing it this way
	std::vector< coloredChar > buffer;

	// malloc/free version - can be used directly for texture
	coloredChar * bufferBase;

};

#endif
