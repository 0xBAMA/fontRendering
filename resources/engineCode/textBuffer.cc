#include "textBuffer.h"

#define OFF ivec2(-1000,-1000)

// ROGUELIKEGAMESTATE
roguelikeGameState::roguelikeGameState () {
	auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
	auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();
	fnFractal->SetSource( fnSimplex );
	fnFractal->SetOctaveCount( 4 );
	fnGenerator = fnFractal;
}
void roguelikeGameState::moveCharacterRight () {
	if ( ChunkyNoise( ivec2( 1, 0 ) ) <= 0 ) {
		playerLocation.x++;
	}
}
void roguelikeGameState::moveCharacterLeft () {
	if ( ChunkyNoise( ivec2( -1, 0 ) ) <= 0 ) {
		playerLocation.x--;
	}
}
// halving movement speed on the y, due to the size 1x2 scaling of the tiles
void roguelikeGameState::moveCharacterUp () {
	static bool toggle = false;
	if ( toggle && ChunkyNoise( ivec2( 0, -1 ) ) <= 0 )
		playerLocation.y--;
	toggle = !toggle;
}
void roguelikeGameState::moveCharacterDown () {
	static bool toggle = false;
	if ( toggle && ChunkyNoise( ivec2( 0, 1 ) ) <= 0 )
		playerLocation.y++;
	toggle = !toggle;
}
int roguelikeGameState::ChunkyNoise ( ivec2 offset ) {
	int seed = 42069;
	vec2 position = ( vec2( playerLocation ) + vec2( offset ) ) * scaleFactor;
	return static_cast< int >( fnGenerator->GenSingle2D( position.x, position.y * 2.0f, seed ) * 10.0 );
}
bool roguelikeGameState::Update () {
	static ivec2 previousPlayerLocation = ivec2( -1, -1 );
	if ( playerLocation != previousPlayerLocation ) {
		previousPlayerLocation = playerLocation;
		PrepareDisplayVector();
		return true;
	}
	return false;
}
void roguelikeGameState::PrepareDisplayVector () {
	// construct 2d representation in the displayString
	displayVector.clear();
	displayVector.reserve( displaySize.x * displaySize.y );
	// const ivec2 offset = ( displaySize / 2u );
	const ivec2 offset = playerDisplayLocation - ivec2( displayBase );
	const unsigned char fills[ 5 ] = { FILL_0, FILL_25, FILL_50, FILL_75, FILL_100 };
	for ( unsigned int y = 0; y <= displaySize.y; y++ ) {
		for ( unsigned int x = 0; x < displaySize.x; x++ ) {
			displayVector.push_back( cChar( GREEN, fills[ std::clamp( ChunkyNoise( ivec2( x, y ) - offset ), 0, 4 ) ] ) );
		}
	}
}
// recursive symmetric shadowcasting based on this implementation
// https://www.albertford.com/shadowcasting/
void roguelikeGameState::MarkVisible ( ivec2 offset ) {
	ivec2 location = playerDisplayLocation - ivec2( displayBase ) + offset;
	unsigned int index = location.x + displaySize.x * location.y;
	if( index < lighting.size() ) {
		float distanceSqr = length( vec2( offset ) );
		distanceSqr = distanceSqr * distanceSqr;
		float val = 100.0f / ( distanceSqr );
		// val = val * val;
		lighting[ index ] = val;
	}
}
bool roguelikeGameState::IsObstruction ( ivec2 offset ) {
	if ( offset == OFF )
		return false;
	return ChunkyNoise( offset ) > 0;
}
glm::mat2x2 rotate2D ( float r ) {
  return glm::mat2x2( cos( r ), sin( r ), -sin( r ), cos( r ) );
}
void roguelikeGameState::DoLightingRays () {
	lighting.clear();
	lighting.resize( displaySize.x * displaySize.y, 0.0 );
	const float numStepsRound = 300.0f;
	const float stepSize = 0.45f;

	std::random_device r;
	std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
	auto gen = std::mt19937_64( s );
	std::uniform_real_distribution< float > dist( 0.0f, 1.0f );

	for ( float r = 0.0f; r < numStepsRound; r += 1.0f ) {
		float distance = 0.0f;
		for ( float s = 0.0f; s < 45.0f; s += 1.0f ) {
			vec2 direction = rotate2D( r * ( ( 2.0f * pi ) / numStepsRound ) ) * vec2( 0.0f, -1.0f );
			distance += dist( gen );
			ivec2 queryLocation = ivec2( direction * distance );
			MarkVisible( queryLocation );
			if ( IsObstruction( queryLocation ) ) {
				break;
			}
		}
	}
}

// Layer
void Layer::ClearBuffer () {
	size_t numBytes = sizeof( cChar ) * bufferSize.x * bufferSize.y;
	memset( ( void * ) bufferBase, 0, numBytes );
}
cChar Layer::GetCharAt ( uvec2 position ) {
	if ( position.x < bufferSize.x && position.y < bufferSize.y ) // >= 0 is implicit with unsigned
		return *( bufferBase + sizeof( cChar ) * ( position.x + position.y * bufferSize.x ) );
	else
		return cChar();
}
void Layer::WriteCharAt ( uvec2 position, cChar c ) {
	if ( position.x < bufferSize.x && position.y < bufferSize.y ) {
		int index = position.x + position.y * bufferSize.x;
		bufferBase[ index ] = c;
	}
}
void Layer::WriteString ( uvec2 min, uvec2 max, std::string str, ivec3 color ) {
	bufferDirty = true;
	uvec2 cursor = min;
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
			WriteCharAt( cursor, cChar( color, ( unsigned char )( c ) ) );
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
void Layer::WriteCCharVector ( uvec2 min, uvec2 max, std::vector< cChar > vec ) {
	bufferDirty = true;
	uvec2 cursor = min;
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
void Layer::WriteLightVector ( uvec2 min, uvec2 max, std::vector< float > vec ) {
	bufferDirty = true;
	uvec2 cursor = min;
	for ( unsigned int i = 0; i < vec.size(); i++ ) {
		cChar c = cChar( ivec3( vec3( GOLD ) * std::clamp( vec[ i ], 0.0f, 1.0f ) ), FILL_100 );
		const bool lit = ( vec[ i ] >= 0.0 );
		if ( lit )
			WriteCharAt( cursor, c );
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
void Layer::DrawRandomChars ( int n ) {
	bufferDirty = true;
	std::random_device r;
	std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
	auto gen = std::mt19937_64( s );
	std::uniform_int_distribution< unsigned char > cDist( 0, 255 );
	std::uniform_int_distribution< unsigned int > xDist( 0, bufferSize.x - 1 );
	std::uniform_int_distribution< unsigned int > yDist( 0, bufferSize.y - 1 );
	for ( int i = 0; i < n; i++ )
		WriteCharAt( uvec2( xDist( gen ), yDist( gen ) ), cChar( ivec3( cDist( gen ), cDist( gen ), cDist( gen ) ), cDist( gen ) ) );
}
void Layer::DrawDoubleFrame ( uvec2 min, uvec2 max, ivec3 color ) {
	bufferDirty = true;
	WriteCharAt( min, cChar( color, TOP_LEFT_DOUBLE_CORNER ) );
	WriteCharAt( uvec2( max.x, min.y ), cChar( color, TOP_RIGHT_DOUBLE_CORNER ) );
	WriteCharAt( uvec2( min.x, max.y ), cChar( color, BOTTOM_LEFT_DOUBLE_CORNER ) );
	WriteCharAt( max, cChar( color, BOTTOM_RIGHT_DOUBLE_CORNER ) );
	for( unsigned int x = min.x + 1; x < max.x; x++  ){
		WriteCharAt( uvec2( x, min.y ), cChar( color, HORIZONTAL_DOUBLE ) );
		WriteCharAt( uvec2( x, max.y ), cChar( color, HORIZONTAL_DOUBLE ) );
	}
	for( unsigned int y = min.y + 1; y < max.y; y++  ){
		WriteCharAt( uvec2( min.x, y ), cChar( color, VERTICAL_DOUBLE ) );
		WriteCharAt( uvec2( max.x, y ), cChar( color, VERTICAL_DOUBLE ) );
	}
}
void Layer::DrawSingleFrame ( uvec2 min, uvec2 max, ivec3 color ) {
	bufferDirty = true;
	WriteCharAt( min, cChar( color, TOP_LEFT_SINGLE_CORNER ) );
	WriteCharAt( uvec2( max.x, min.y ), cChar( color, TOP_RIGHT_SINGLE_CORNER ) );
	WriteCharAt( uvec2( min.x, max.y ), cChar( color, BOTTOM_LEFT_SINGLE_CORNER ) );
	WriteCharAt( max, cChar( color, BOTTOM_RIGHT_SINGLE_CORNER ) );
	for( unsigned int x = min.x + 1; x < max.x; x++  ){
		WriteCharAt( uvec2( x, min.y ), cChar( color, HORIZONTAL_SINGLE ) );
		WriteCharAt( uvec2( x, max.y ), cChar( color, HORIZONTAL_SINGLE ) );
	}
	for( unsigned int y = min.y + 1; y < max.y; y++  ){
		WriteCharAt( uvec2( min.x, y ), cChar( color, VERTICAL_SINGLE ) );
		WriteCharAt( uvec2( max.x, y ), cChar( color, VERTICAL_SINGLE ) );
	}
}
void Layer::DrawCurlyScroll ( uvec2 start, unsigned int length, ivec3 color ) {
	bufferDirty = true;
	WriteCharAt( start, cChar( color, CURLY_SCROLL_TOP ) );
	for ( unsigned int i = 1; i < length; i++ ) {
		WriteCharAt( start + uvec2( 0, i ), cChar ( color, CURLY_SCROLL_MIDDLE ) );
	}
	WriteCharAt( start + uvec2( 0, length ), cChar( color, CURLY_SCROLL_BOTTOM ) );
}
void Layer::DrawRectRandom ( uvec2 min, uvec2 max, ivec3 color ) {
	bufferDirty = true;
	std::random_device r;
	std::seed_seq s{ r(), r(), r(), r(), r(), r(), r(), r(), r() };
	auto gen = std::mt19937_64( s );
	std::uniform_int_distribution< unsigned char > fDist( 0, 4 );
	const unsigned char fills[ 5 ] = { FILL_0, FILL_25, FILL_50, FILL_75, FILL_100 };

	for( unsigned int x = min.x; x <= max.x; x++ ) {
		for( unsigned int y = min.y; y <= max.y; y++ ) {
			WriteCharAt( uvec2( x, y ), cChar( color, fills[ fDist( gen ) ] ) );
		}
	}
}
void Layer::DrawRectConstant ( uvec2 min, uvec2 max, cChar c ) {
	for( unsigned int x = min.x; x <= max.x; x++ ) {
		for( unsigned int y = min.y; y <= max.y; y++ ) {
			WriteCharAt( uvec2( x, y ), c );
		}
	}
}
void Layer::BindAndSendUniforms () {
	glUniform2i( offsetUniformLocation, bufferOffset.x, bufferOffset.y );
	glUniform1f( alphaUniformLocation, alpha );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, textureHandle );
	if ( bufferDirty ) {
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, bufferSize.x, bufferSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, bufferBase );
		bufferDirty = false;
	}
	glBindImageTexture( 1, textureHandle, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8UI );
}

// TextBufferManager
TextBufferManager::TextBufferManager ( uvec2 screenDimensions ) {
	displaySize = screenDimensions;
}
TextBufferManager::~TextBufferManager () {

}
void TextBufferManager::Populate () {
	uvec2 baseSize = uvec2( numCharsWidthDefault - 2 * numCharsBorderX, numCharsHeightDefault - 2 * numCharsBorderY );
	uvec2 borderAmount = ivec2( numCharsBorderX, numCharsBorderY );

	// create layers
	layers.push_back( Layer( baseSize, borderAmount, 1.0 ) ); // noise bg
	layers.push_back( Layer( baseSize, borderAmount, 1.0 ) ); // lighting
	layers.push_back( Layer( baseSize, borderAmount, 1.0 ) ); // character
	layers.push_back( Layer( uvec2( 53, 53 ), ivec2( 139, 4 ), 1.0 ) );
	layers.push_back( Layer( uvec2( 52, 51 ), ivec2( 140, 5 ), 1.0 ) );

	rgd.displayBase = uvec2( 3, 2 );
	rgd.displaySize = baseSize - ( 2u * rgd.displayBase );
	rgd.playerDisplayLocation = rgd.displayBase + ( rgd.displaySize / 2u );
	rgd.playerDisplayLocation.x -= 25;

	// base layer
	layers[ 1 ].ClearBuffer();
	layers[ 1 ].DrawDoubleFrame( uvec2( 0, 0 ), baseSize - uvec2( 1, 1 ), GOLD );

	layers[ 0 ].ClearBuffer();

	layers[ 2 ].ClearBuffer();
	layers[ 2 ].WriteCharAt( rgd.playerDisplayLocation, cChar( WHITE, 2 ) );

	layers[ 3 ].ClearBuffer();
	layers[ 3 ].DrawRectConstant( uvec2( 0, 1 ), layers[ 3 ].bufferSize - uvec2( 1, 2 ), cChar( GOLD, FILL_25 ) );
	layers[ 3 ].DrawCurlyScroll( uvec2( 0, 0 ), layers[ 3 ].bufferSize.y - 1, GOLD );

	layers[ 4 ].ClearBuffer();

	for ( auto& l : layers ) {
		l.offsetUniformLocation = offsetUniformLocation;
		l.alphaUniformLocation = alphaUniformLocation;
	}
}
void TextBufferManager::Update () {
	glUniform2i( displayUniformLocation, displaySize.x, displaySize.y );
	rgd.Update();
	// rgd.DoLighting();
	rgd.DoLightingRays();
	layers[ 1 ].WriteCCharVector( rgd.displayBase, rgd.displaySize + rgd.displayBase, rgd.displayVector ) ;
	layers[ 0 ].ClearBuffer();
	layers[ 0 ].WriteLightVector( rgd.displayBase, rgd.displaySize + rgd.displayBase, rgd.lighting );
	layers[ 4 ].DrawRandomChars( 22 );
}
void TextBufferManager::DrawAllLayers () {
	for ( unsigned int i = 0; i < layers.size(); i++ ) {
		layers[ i ].BindAndSendUniforms();
		glDrawArrays( GL_TRIANGLES, 0, 3 );
	}
}
