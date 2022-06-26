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
		, bufferSize( x - numCharsBorderX * 2, y - numCharsBorderY * 2 )
		, offset( numCharsBorderX, numCharsBorderY ) {

			ResetBuffer();
			Draw();

	}

	// size of the buffer
	glm::uvec2 dimensions;	// size of display
	glm::uvec2 bufferSize;	// size of the buffer
	glm::ivec2 offset;			// offset of the buffer within the display
	bool updateFlag = false;// is there new data to send?
	bool redrawFlag = false;// is there new data to draw?
	void Update () {
		// check for buffer resize
		static glm::uvec2 lastUpdateBufferSize = bufferSize;
		if ( bufferSize != lastUpdateBufferSize ) {
			lastUpdateBufferSize = bufferSize;
			ResetBuffer();
		}
		if( redrawFlag ) {
			ZeroBuffer();
			Draw();
		}
		// send the data to the GPU
		if ( updateFlag ) {
			ResendData();
		}
	}

	void Draw () {
		DrawDoubleFrame( glm::uvec2( 0, 0 ), glm::uvec2( bufferSize.x - 1, bufferSize.y - 1 ), GOLD );
		DrawDoubleFrame( glm::uvec2( 10, 10 ), glm::uvec2( 65, 24 ), BLUE );
		DrawRandomChars( 1000 );
		DrawDoubleFrame( glm::uvec2( 50, 18 ), glm::uvec2( 100, 36 ), GOLD );
		DrawRandomChars( 1000 );
		DrawRectRandom( glm::uvec2( 65, 32 ), glm::uvec2( 175, 45 ), BLUE );
		DrawRandomChars( 100 );
		WriteString( glm::uvec2( 100, 10 ), glm::uvec2( 180, 30 ), std::string( "This is a string being written, get written, string - string string string string string string string Get Stringed, idiot string go and get stringed - This is a string being written, get written, string - string string string string string string string Get Stringed, idiot string go and get stringed - This is a string being written, get written, string - string string string string string string string Get Stringed, idiot string go and get stringed - This is a string being written, get written, string - string string string string string string string Get Stringed, idiot string go and get stringed - This is a string being written, get written, string - string string string string string string string Get Stringed, idiot string go and get stringed" ), GOLD );
	}

	void DrawRandomChars ( int n ) {
		std::random_device r;
		std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
		auto gen = std::mt19937_64( s );
		std::uniform_int_distribution< unsigned char > cDist( 0, 255 );
		std::uniform_int_distribution< unsigned int > xDist( 0, bufferSize.x - 1 );
		std::uniform_int_distribution< unsigned int > yDist( 0, bufferSize.y - 1 );
		for ( int i = 0; i < n; i++ )
			WriteCharAt( glm::uvec2( xDist( gen ), yDist( gen ) ), coloredChar( glm::ivec3( cDist( gen ), cDist( gen ), cDist( gen ) ), cDist( gen ) ) );
	}

	void DrawDoubleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
		WriteCharAt( min, coloredChar( color, TOP_LEFT_DOUBLE_CORNER ) );
		WriteCharAt( glm::uvec2( max.x, min.y ), coloredChar( color, TOP_RIGHT_DOUBLE_CORNER ) );
		WriteCharAt( glm::uvec2( min.x, max.y ), coloredChar( color, BOTTOM_LEFT_DOUBLE_CORNER ) );
		WriteCharAt( max, coloredChar( color, BOTTOM_RIGHT_DOUBLE_CORNER ) );
		for( unsigned int x = min.x + 1; x < max.x; x++  ){
			WriteCharAt( glm::uvec2( x, min.y ), coloredChar( color, HORIZONTAL_DOUBLE ) );
			WriteCharAt( glm::uvec2( x, max.y ), coloredChar( color, HORIZONTAL_DOUBLE ) );
		}
		for( unsigned int y = min.y + 1; y < max.y; y++  ){
			WriteCharAt( glm::uvec2( min.x, y ), coloredChar( color, VERTICAL_DOUBLE ) );
			WriteCharAt( glm::uvec2( max.x, y ), coloredChar( color, VERTICAL_DOUBLE ) );
		}
	}

	void DrawRectRandom ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
		std::random_device r;
		std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
		auto gen = std::mt19937_64( s );
		std::uniform_int_distribution< unsigned char > fDist( 0, 4 );
		unsigned char fills[ 5 ] = { FILL_0, FILL_25, FILL_50, FILL_75, FILL_100 };

		for( unsigned int x = min.x; x <= max.x; x++ ) {
			for( unsigned int y = min.y; y <= max.y; y++ ) {
				WriteCharAt( glm::uvec2( x, y ), coloredChar( color, fills[ fDist( gen ) ] ) );
			}
		}
	}

	void WriteString ( glm::uvec2 min, glm::uvec2 max, std::string str, glm::ivec3 color ) {
		glm::uvec2 cursor = min;
		for ( auto c : str ) {
			WriteCharAt( cursor, coloredChar( color, ( unsigned char )( c ) ) );
			cursor.x++;
			if ( cursor.x >= max.x ) {
				cursor.y++;
				cursor.x = min.x;
				if ( cursor.y >= max.y ) {
					break;
				}
			}
		}
	}

	void ResetBuffer () {
		if ( bufferBase != nullptr )
			free( bufferBase );	// deallocate the memory for the buffer
		size_t numBytes = sizeof( coloredChar ) * bufferSize.x * bufferSize.y;
		bufferBase = ( coloredChar * ) malloc( numBytes ); // allocate a new buffer of the new size
		updateFlag = true;
	}

	void ZeroBuffer () {
		size_t numBytes = sizeof( coloredChar ) * bufferSize.x * bufferSize.y;
		memset( ( void * ) bufferBase, 0, numBytes );
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
