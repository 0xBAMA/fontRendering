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

struct worldSample {
	bool obstruction = false;
	coloredChar representation;
};

struct worldState {
public:
	worldState ();
	float GetNoise ( glm::vec2 position );
	FastNoise::SmartNode<> fnGenerator;
};

class roguelikeGameDisplay {
public:
	bool Update ();
	// centerpoint of the display
	glm::ivec2 playerLocation = glm::ivec2( 0, 0 );
	// access from above for the display - need to figure out colors, too
	std::string displayString;
	// allows the displayString to be used directly by the textBuffer
	glm::uvec2 displayBase = glm::uvec2( 4, 2 );
	glm::uvec2 displaySize = glm::uvec2( 182, 53 );
private:
	float scaleFactor = 0.01f;
	void PrepareDisplayString ();
	worldState ws;
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

	~textBuffer(){
		free( bufferBase );
	}

	roguelikeGameDisplay rgd;

	// size of the buffer
	glm::uvec2 dimensions;	// size of display
	glm::uvec2 bufferSize;	// size of the buffer
	glm::ivec2 offset;			// offset of the buffer within the display
	bool updateFlag = false;// is there new data to send?
	bool redrawFlag = false;// is there new data to draw?
	void Update ();
	void Draw ();
	void DrawRandomChars ( int n );
	void DrawDoubleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color );
	void DrawSingleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color );
	void DrawRectRandom ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color );
	void WriteString ( glm::uvec2 min, glm::uvec2 max, std::string str, glm::ivec3 color );

	void moveCharacterRight ();
	void moveCharacterLeft ();
	void moveCharacterDown ();
	void moveCharacterUp ();

	void ResetBuffer ();
	void ZeroBuffer ();

	coloredChar GetCharAt ( glm::uvec2 position );
	void WriteCharAt ( glm::uvec2 position, coloredChar c );

	// send the data to the GPU
	GLint dataTexture;	// texture handle for the GPU
	void ResendData ();

private:
	// malloc/free version - can be used directly for texture
	coloredChar * bufferBase = nullptr;
};

#endif
