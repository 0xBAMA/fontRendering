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
	coloredChar( glm::ivec3 color, unsigned char c ) {
		data[ 0 ] = color.x;
		data[ 1 ] = color.y;
		data[ 2 ] = color.z;
		data[ 3 ] = c;
	}

};

class textBuffer {
public:
	textBuffer( unsigned int x, unsigned int y )
		: dimensions( x, y )
		, bufferSize( 80, 35 )
		, offset( 5, 2 ) {
			ResetBuffer();

			std::random_device r;
			std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
			auto gen = std::mt19937_64( s );
			std::uniform_int_distribution< unsigned char > cDist( 0, 255 );
			std::uniform_int_distribution< unsigned int > xDist( 0, bufferSize.x - 1 );
			std::uniform_int_distribution< unsigned int > yDist( 0, bufferSize.y - 1 );
			for ( int i = 0; i < 10000; i++ )
				WriteCharAt( glm::uvec2( xDist( gen ), yDist( gen ) ), coloredChar( glm::ivec3( cDist( gen ), cDist( gen ), cDist( gen ) ), cDist( gen ) ) );


			// WriteCharAt( glm::uvec2( 10, 10 ), coloredChar( 'c' ) );

			// std::string test( "this is a test string, get tested, idiot" );
			// int i = 0;
			// for ( auto& c : test ) {
			// 	// WriteCharAt( glm::uvec2( i++, 15 ), coloredChar( glm::ivec3( 255, 128, 128 ), c ) );
			// 	WriteCharAt( glm::uvec2( i++, 15 ), coloredChar( c ) );
			// }
	}

	// size of the buffer
	glm::uvec2 dimensions;	// size of display
	glm::uvec2 bufferSize;	// size of the buffer
	glm::ivec2 offset;			// offset of the buffer within the display
	bool updateFlag = false;// is there new data to send?
	void Update () {
		// check for buffer resize
		static glm::uvec2 lastUpdateBufferSize = bufferSize;
		if ( bufferSize != lastUpdateBufferSize ) {
			lastUpdateBufferSize = bufferSize;
			ResetBuffer();
		}
		// send the data to the GPU
		if ( updateFlag ) {
			ResendData();
		}
	}

	void ResetBuffer () {
		if ( bufferBase != nullptr )
			free( bufferBase );	// deallocate the memory for the buffer
		size_t numBytes = sizeof( coloredChar ) * bufferSize.x * bufferSize.y;
		bufferBase = ( coloredChar * ) malloc( numBytes ); // allocate a new buffer of the new size
		memset( ( void * ) bufferBase, 0, numBytes );
		updateFlag = true;
	}

	coloredChar GetCharAt ( glm::uvec2 position ) {
		if ( position.x >= 0 && position.x < bufferSize.x && position.y >= 0 && position.y < bufferSize.y )
			return *( bufferBase + sizeof( coloredChar ) * ( position.x + position.y * bufferSize.x ) );
		else
			return coloredChar();
	}

	void WriteCharAt ( glm::uvec2 position, coloredChar c ) {
		if ( position.x >= 0 && position.x < bufferSize.x && position.y >= 0 && position.y < bufferSize.y ) {
			int index = position.x + position.y * bufferSize.x;
			bufferBase[ index ] = c;
		}
	}

	void WriteString ( glm::uvec2 position, unsigned char * str, unsigned int count, glm::ivec3 color ) {
		updateFlag = true;
		unsigned int base = position.x + position.y * bufferSize.x;
		if ( base + count >= ( bufferSize.x * bufferSize.y ) ) {
			cout << "string runs off the end of the buffer - aborting" << endl;
			return;
		}

		for ( unsigned int i = base; i < base + count; i++ ) {
			bufferBase[ i ] = coloredChar( color, *( str + count ) );
		}
	}

	// send the data to the GPU
	GLint dataTexture;	// texture handle for the GPU
	void ResendData() {
		updateFlag = false;
		glBindTexture( GL_TEXTURE_2D, dataTexture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, bufferSize.x, bufferSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferBase );
	}

private:
	// malloc/free version - can be used directly for texture
	coloredChar * bufferBase = nullptr;
};

#endif
