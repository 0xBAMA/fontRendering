#include "textBuffer.h"

// worldState
worldState::worldState () {
	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
	fnFractal->SetSource( fnSimplex );
	fnFractal->SetOctaveCount( 4 );
	fnGenerator = fnFractal;
}
float worldState::GetNoise ( glm::vec2 position ) {
	int seed = 42069;
	return ( fnGenerator->GenSingle2D( position.x, position.y * 2.0f, seed ) + 1.0f ) * 0.5f;
}

// ROGUELIKEGAMEDISPLAY
bool roguelikeGameDisplay::Update () {
	static glm::ivec2 previousPlayerLocation = glm::ivec2( -1, -1 );
	if ( playerLocation != previousPlayerLocation ) {
		previousPlayerLocation = playerLocation;
		PrepareDisplayVector();
		return true;
	}
	return false;
}
void roguelikeGameDisplay::PrepareDisplayVector () {
	// construct 2d representation in the displayString
	displayVector.clear();
	displayVector.reserve( displaySize.x * displaySize.y );
	const glm::vec2 offset = ( displaySize / 2u );
	const unsigned char fills[ 5 ] = { FILL_0, FILL_25, FILL_50, FILL_75, FILL_100 };
	for( unsigned int y = 0; y < displaySize.y; y++ ) {
		for( unsigned int x = 0; x < displaySize.x; x++ ) {
			glm::vec2 samplePoint = ( glm::vec2( playerLocation ) + glm::vec2( x, y ) - offset ) * scaleFactor;
			displayVector.push_back( coloredChar( GREEN, fills[ std::clamp( static_cast< int >( ws.GetNoise( samplePoint ) * 8 - 1 ), 0, 4 ) ] ) );
		}
	}
}
// TODO: character movement functions need to check for obstruction
void roguelikeGameDisplay::moveCharacterRight () {
	playerLocation.x++;
}
void roguelikeGameDisplay::moveCharacterLeft () {
	playerLocation.x--;
}
// halving movement speed on the y, due to the size of the tiles
void roguelikeGameDisplay::moveCharacterUp () {
	static bool toggle = false;
	if ( toggle )
		playerLocation.y--;
	toggle = !toggle;
}
void roguelikeGameDisplay::moveCharacterDown () {
	static bool toggle = false;
	if ( toggle )
		playerLocation.y++;
	toggle = !toggle;
}

// Layer
Layer::Layer ( glm::uvec2 bSize, glm::ivec2 bOffset ) {
	bufferSize = bSize;
	bufferOffset = bOffset;
	glGenTextures( 1, &textureHandle ); // get a new texture handle from OpenGL
	// allocate a new buffer of the specified size
	size_t numBytes = sizeof( coloredChar ) * bufferSize.x * bufferSize.y;
	bufferBase = ( coloredChar * ) malloc( numBytes );
	bufferDirty = true; // data will need to be resent next frame
}
Layer::~Layer () {
	if ( bufferBase != nullptr )
		free( bufferBase );	// deallocate the memory for the buffer
}
void Layer::ClearBuffer () {
	size_t numBytes = sizeof( coloredChar ) * bufferSize.x * bufferSize.y;
	memset( ( void * ) bufferBase, 0, numBytes );
}
coloredChar Layer::GetCharAt ( glm::uvec2 position ) {
	if ( position.x < bufferSize.x && position.y < bufferSize.y ) // >= 0 is implicit with unsigned
		return *( bufferBase + sizeof( coloredChar ) * ( position.x + position.y * bufferSize.x ) );
	else
		return coloredChar();
}
void Layer::WriteCharAt ( glm::uvec2 position, coloredChar c ) {
	if ( position.x < bufferSize.x && position.y < bufferSize.y ) {
		int index = position.x + position.y * bufferSize.x;
		bufferBase[ index ] = c;
	}
}
void Layer::WriteString ( glm::uvec2 min, glm::uvec2 max, std::string str, glm::ivec3 color ) {
	bufferDirty = true;
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
void Layer::WriteColoredCharVector ( glm::uvec2 min, glm::uvec2 max, std::vector< coloredChar > vec ) {
	bufferDirty = true;
	glm::uvec2 cursor = min;
	for ( unsigned int i = 0; i < vec.size(); i++ ) {
		if ( vec[ i ].data[ 4 ] == '\t' ) {
			cursor.x += 2;
		} else if ( vec[ i ].data[ 4 ] == '\n' ) {
			cursor.y++;
			cursor.x = min.x;
			if ( cursor.y >= max.y ) {
				break;
			}
		} else if ( vec[ i ].data[ 4 ] == 0 ) { // special no-write character
			cursor.x++;
		} else {
			WriteCharAt( cursor, vec[ i ] );
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
void Layer::DrawRandomChars ( int n ) {
	bufferDirty = true;
	std::random_device r;
	std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
	auto gen = std::mt19937_64( s );
	std::uniform_int_distribution< unsigned char > cDist( 0, 255 );
	std::uniform_int_distribution< unsigned int > xDist( 0, bufferSize.x - 1 );
	std::uniform_int_distribution< unsigned int > yDist( 0, bufferSize.y - 1 );
	for ( int i = 0; i < n; i++ )
		WriteCharAt( glm::uvec2( xDist( gen ), yDist( gen ) ), coloredChar( glm::ivec3( cDist( gen ), cDist( gen ), cDist( gen ) ), cDist( gen ) ) );
}
void Layer::DrawDoubleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
	bufferDirty = true;
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
void Layer::DrawSingleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
	bufferDirty = true;
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
void Layer::DrawCurlyScroll ( glm::uvec2 start, unsigned int length, glm::ivec3 color ) {
	bufferDirty = true;
	WriteCharAt( start, coloredChar( color, CURLY_SCROLL_TOP ) );
	for ( unsigned int i = 1; i < length; i++ ) {
		WriteCharAt( start + glm::uvec2( 0, i ), coloredChar ( color, CURLY_SCROLL_MIDDLE ) );
	}
	WriteCharAt( start + glm::uvec2( 0, length ), coloredChar( color, CURLY_SCROLL_BOTTOM ) );
}
void Layer::DrawRectRandom ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color ) {
	bufferDirty = true;
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
void Layer::BindAndSendUniforms () {
	// glUniform2i( offsetUniformLocation, bufferOffset.x, bufferOffset.y );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, textureHandle );
	if ( bufferDirty ) {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, bufferSize.x, bufferSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferBase );
		bufferDirty = false;
	}
	glBindImageTexture( 1, textureHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );
}


// TextBufferManager
TextBufferManager::TextBufferManager ( glm::uvec2 screenDimensions ) {
	displaySize = screenDimensions;
}
TextBufferManager::~TextBufferManager () {

}
void TextBufferManager::Update () {
	// glUniform2i( displayUniformLocation, displaySize.x, displaySize.y );

}
void TextBufferManager::DrawAllLayers () {
	// iterate through layers
		// call BindAndSendUniforms () to prepare state for layer draw
		// draw the fullscreen triangle
}
