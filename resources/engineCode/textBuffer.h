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

// use these to send the uniforms
	// bufferSize global uniform location
	// bufferOffset global uniform location


// Layer:
	// Members:
		// uvec2 buffer size
		// ivec2 buffer offset
		// GLuint texture handle
		// bool dirty
		// coloredChar * bufferBase

	// Functions:
		// draw random chars
		// draw double frame
		// draw single frame
		// draw curly scroll
		// draw rect random
		// write string
		// write coloredChar vector
		// memset clear
		// get coloredChar at uvec2
		// set coloredChar at uvec2
		// resend data to rebuffer to GPU



class Layer {
public:
	Layer ( glm::uvec2 bSize, glm::ivec2 bOffset );
	~Layer ();

	glm::uvec2 bufferSize;
	glm::ivec2 bufferOffset;
	GLuint textureHandle;
	bool bufferDirty;
	coloredChar * bufferBase = nullptr;

	void ClearBuffer ();
	coloredChar GetCharAt ( glm::uvec2 position );
	void WriteCharAt ( glm::uvec2 position, coloredChar c );
	void WriteString ( glm::uvec2 min, glm::uvec2 max, std::string str, glm::ivec3 color );
	void WriteColoredCharVector ( glm::uvec2 min, glm::uvec2 max, std::vector< coloredChar > vec );
	void DrawRandomChars ( int n );
	void DrawDoubleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color );
	void DrawSingleFrame ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color );
	void DrawCurlyScroll ( glm::uvec2 start, unsigned int length, glm::ivec3 color );
	void DrawRectRandom ( glm::uvec2 min, glm::uvec2 max, glm::ivec3 color );
	void BindAndSendUniforms ();
};


// TextBufferManager:
	// Members:
		// vector of Layers ( in draw order )
		// uvec2 size of the screen, in characters
		// roguelikeGameDisplay object

	// Functions:
		// update function to update each Layer + roguelikeGameDisplay
		// draw function, iterates over Layers
		// calls the rebuffer on any dirty buffers
		// bind texture then fullscreen triangle, no depth test
		// worldSample / worldSample tbd

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
	// access from above for the display - includes colors now
	std::vector< coloredChar > displayVector;
	// allows the displayString to be used directly by the textBuffer
	glm::uvec2 displayBase = glm::uvec2( 4, 2 );
	glm::uvec2 displaySize = glm::uvec2( 182, 53 );

	void moveCharacterRight();
	void moveCharacterLeft();
	void moveCharacterUp();
	void moveCharacterDown();

private:
	float scaleFactor = 0.01f;
	void PrepareDisplayVector ();
	worldState ws;
};
class TextBufferManager {
public:
	glm::uvec2 displaySize;
	std::vector < Layer > layers;
	roguelikeGameDisplay rgd;

	TextBufferManager ( glm::uvec2 screenDimensions);
	~TextBufferManager ();

	void Update ();
	void DrawAllLayers ();

	GLint offsetUniformLocation;
	GLint displayUniformLocation;
};

#endif
