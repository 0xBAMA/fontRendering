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

struct fieldGenerator {
public:
	fieldGenerator() {
		auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
		auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
		fnFractal->SetSource( fnSimplex );
		fnFractal->SetOctaveCount( 4 );
		fnGenerator = fnFractal;
	}

	float GetNoise ( glm::vec2 position ) {
		int seed = 42069;
		return ( fnGenerator->GenSingle2D( position.x, position.y * 2.0f, seed ) + 1.0f ) * 0.5f;
	}

	FastNoise::SmartNode<> fnGenerator;
};

class roguelikeGameDisplay {
public:
	glm::ivec2 playerLocation = glm::ivec2( 0, 0 );

	// access from above for the display
	std::string displayString;
	// allows the displayString to be used directly by the textBuffer
	glm::uvec2 displayBase = glm::uvec2( 4, 2 );
	glm::uvec2 displaySize = glm::uvec2( 182, 53 );

	bool Update () {
		static glm::ivec2 previousPlayerLocation = glm::ivec2( -1, -1 );
		if ( playerLocation != previousPlayerLocation ) {
			previousPlayerLocation = playerLocation;
			PrepareDisplayString();
			return true;
		}
		return false;
	}

private:
	void PrepareDisplayString () {
		// construct 2d representation in the displayString
		displayString = std::string();
		const float scaleFactor = 0.01f;
		const glm::vec2 offset = ( displaySize / 2u );
		const unsigned char fills[ 5 ] = { FILL_0, FILL_25, FILL_50, FILL_75, FILL_100 };
		for( unsigned int y = 0; y < displaySize.y; y++ ) {
			for( unsigned int x = 0; x < displaySize.x; x++ ) {
				glm::vec2 samplePoint = ( glm::vec2( playerLocation ) + glm::vec2( x, y ) - offset ) * scaleFactor;
				displayString += fills[ std::clamp( static_cast< int >( noise.GetNoise( samplePoint ) * 8 - 1 ), 0, 4 ) ];
			}
		}
	}

	fieldGenerator noise;
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

	roguelikeGameDisplay rgd;

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

		// DrawRectRandom( glm::uvec2( 65, 32 ), glm::uvec2( 175, 45 ), BLUE );
		// DrawRandomChars( 100 );

		// update displayString
		if ( rgd.Update() ) {
			// noise field in displayString
			WriteString( rgd.displayBase, rgd.displayBase + rgd.displaySize, rgd.displayString, BLUE );

			// character at center of the noise display
			WriteCharAt( rgd.displayBase + rgd.displaySize / 2u, coloredChar( GOLD, 2 ) );
		}



		// send the data to the GPU
		if ( updateFlag ) {
			ResendData();
		}


	}

	void Draw () {
		DrawDoubleFrame( glm::uvec2( 0, 0 ), glm::uvec2( bufferSize.x - 1, bufferSize.y - 1 ), GOLD );
		// DrawSingleFrame( glm::uvec2( 0, 0 ), glm::uvec2( bufferSize.x - 1, bufferSize.y - 1 ), GOLD );
	}

	void DrawRandomChars ( int n ) {
		updateFlag = true;
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
		updateFlag = true;
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

	void DrawSingleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
		updateFlag = true;
		WriteCharAt( min, coloredChar( color, TOP_LEFT_SINGLE_CORNER ) );
		WriteCharAt( glm::uvec2( max.x, min.y ), coloredChar( color, TOP_RIGHT_SINGLE_CORNER ) );
		WriteCharAt( glm::uvec2( min.x, max.y ), coloredChar( color, BOTTOM_LEFT_SINGLE_CORNER ) );
		WriteCharAt( max, coloredChar( color, BOTTOM_RIGHT_SINGLE_CORNER ) );
		for( unsigned int x = min.x + 1; x < max.x; x++  ){
			WriteCharAt( glm::uvec2( x, min.y ), coloredChar( color, HORIZONTAL_SINGLE ) );
			WriteCharAt( glm::uvec2( x, max.y ), coloredChar( color, HORIZONTAL_SINGLE ) );
		}
		for( unsigned int y = min.y + 1; y < max.y; y++  ){
			WriteCharAt( glm::uvec2( min.x, y ), coloredChar( color, VERTICAL_SINGLE ) );
			WriteCharAt( glm::uvec2( max.x, y ), coloredChar( color, VERTICAL_SINGLE ) );
		}
	}

	void DrawRectRandom ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
		updateFlag = true;
		std::random_device r;
		std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
		auto gen = std::mt19937_64( s );
		std::uniform_int_distribution< unsigned char > fDist( 0, 4 );
		const unsigned char fills[ 5 ] = { FILL_0, FILL_25, FILL_50, FILL_75, FILL_100 };

		for( unsigned int x = min.x; x <= max.x; x++ ) {
			for( unsigned int y = min.y; y <= max.y; y++ ) {
				WriteCharAt( glm::uvec2( x, y ), coloredChar( color, fills[ fDist( gen ) ] ) );
			}
		}
	}

	void WriteString ( glm::uvec2 min, glm::uvec2 max, std::string str, glm::ivec3 color ) {
		updateFlag = true;
		glm::uvec2 cursor = min;
		for ( auto c : str ) {
			if ( c == '\t' ) {
				cursor.x += 2;
			} else if ( c == '\n' ) {
				cursor.y++;
				cursor.x = min.x;
				if ( cursor.y >= max.y ) {
					break;
				}
			} else if ( c == 0 ) { // null character, don't draw anything - can use 32 aka space to overwrite with blank
				cursor.x++;
			} else {
				WriteCharAt( cursor, coloredChar( color, ( unsigned char )( c ) ) );
				cursor.x++;
			}
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
		if ( position.x < bufferSize.x && position.y < bufferSize.y ) // >= 0 is implicit with unsigned
			return *( bufferBase + sizeof( coloredChar ) * ( position.x + position.y * bufferSize.x ) );
		else
			return coloredChar();
	}

	void WriteCharAt ( glm::uvec2 position, coloredChar c ) {
		if ( position.x < bufferSize.x && position.y < bufferSize.y ) {
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

	~textBuffer(){
		free( bufferBase );
	}

private:
	// malloc/free version - can be used directly for texture
	coloredChar * bufferBase = nullptr;
};

#endif
