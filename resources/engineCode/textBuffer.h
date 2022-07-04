#include "includes.h"
#ifndef TEXTBUFFER
#define TEXTBUFFER

#include "extendedASCIIdefines.h"
using glm::ivec2;
using glm::ivec3;
using glm::vec2;
using glm::vec3;
using glm::uvec2;

struct cChar {
	unsigned char data[ 4 ] = { 255, 255, 255, 0 };
	cChar() {}
	cChar( unsigned char c ) {
		data[ 3 ] = c;
	}
	cChar( glm::ivec3 color, unsigned char c ) {
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
		// cChar * bufferBase

	// Functions:
		// draw random chars
		// draw double frame
		// draw single frame
		// draw curly scroll
		// draw rect random
		// write string
		// write cChar vector
		// memset clear
		// get cChar at uvec2
		// set cChar at uvec2
		// resend data to rebuffer to GPU

class Layer {
public:
	Layer ( uvec2 bSize, ivec2 bOffset, float a ) : bufferSize( bSize ), bufferOffset( bOffset ), alpha( a ) {
		glGenTextures( 1, &textureHandle ); // get a new texture handle from OpenGL
		// allocate a new buffer of the specified size
		size_t numBytes = sizeof( cChar ) * bufferSize.x * bufferSize.y;
		bufferBase = ( cChar * ) malloc( numBytes );
		bufferDirty = true; // data will need to be resent next frame
	}
	~Layer () { // tbd
		// if ( bufferBase != nullptr )
		// 	free( bufferBase );	// deallocate the memory for the buffer
	}

	// buffer parameters
	uvec2 bufferSize;
	ivec2 bufferOffset;
	float alpha;

	GLuint textureHandle;
	bool bufferDirty;
	cChar * bufferBase = nullptr;
	GLint offsetUniformLocation;
	GLint alphaUniformLocation;

	void ClearBuffer ();
	cChar GetCharAt ( uvec2 position );
	void WriteCharAt ( uvec2 position, cChar c );
	void WriteString ( uvec2 min, uvec2 max, std::string str, ivec3 color );
	void WriteCCharVector ( uvec2 min, uvec2 max, std::vector< cChar > vec );
	void WriteLightVector ( uvec2 min, uvec2 max, std::vector< float > vec );
	void DrawRandomChars ( int n );
	void DrawDoubleFrame ( uvec2 min, uvec2 max, ivec3 color );
	void DrawSingleFrame ( uvec2 min, uvec2 max, ivec3 color );
	void DrawCurlyScroll ( uvec2 start, unsigned int length, ivec3 color );
	void DrawRectRandom ( uvec2 min, uvec2 max, ivec3 color );
	void DrawRectConstant ( uvec2 min, uvec2 max, cChar c );
	void BindAndSendUniforms ();
};

// TextBufferManager:
	// Members:
		// vector of Layers ( in draw order )
		// uvec2 size of the screen, in characters
		// roguelikeGameState object

	// Functions:
		// update function to update each Layer + roguelikeGameState
		// draw function, iterates over Layers
		// calls the rebuffer on any dirty buffers
		// bind texture then fullscreen triangle, no depth test
		// worldSample / worldSample tbd

// used by lighting
struct Row {
	float roundUp( float in ) { return std::floor( in + 0.5f ); }
	float roundDown( float in ) { return std::ceil( in - 0.5 ); }
	Row ( int d, float s, float e, ivec2 x, ivec2 y ) : depth( d ), startSlope( s ), endSlope( e ), xBasis( x ), yBasis( y ) {
		minColumn = roundUp( depth * startSlope );
		maxColumn = roundDown( depth * endSlope );
		numTiles = maxColumn - minColumn;
	}
	int depth;
	float startSlope;
	float endSlope;
	ivec2 xBasis;
	ivec2 yBasis;
	int minColumn;
	int maxColumn;
	int numTiles;
	ivec2 GetTile ();
	Row Next() {
		return Row( depth+1, startSlope, endSlope, xBasis, yBasis );
	}
	bool IsSymmetric( int i ) {
		int column = minColumn + i;
		return ( column >= depth * startSlope && column <= depth * endSlope );
	}
};

class roguelikeGameState {
public:
	roguelikeGameState ();
	bool Update ();
	void DoLighting ();
	void DoLightingRays ();

	// centerpoint of the display:
		// location is offset in world
	ivec2 playerLocation = ivec2( 0, 0 );
		// display location is offset on display texture ( constant for a given displaySize )
	ivec2 playerDisplayLocation;

	// access from above for the display - includes colors now
	std::vector< cChar > displayVector;

	// lighting state, kept with the same mapping as the above vector
	std::vector< float > lighting;
	void MarkVisible ( ivec2 offset );
	void MarkInvisible ( ivec2 offset );
	bool IsObstruction ( ivec2 offset );
	void RecursiveScan ( Row r );

	// allows the displayString to be used directly by the textBuffer
	uvec2 displayBase;
	uvec2 displaySize;

	void moveCharacterRight();
	void moveCharacterLeft();
	void moveCharacterUp();
	void moveCharacterDown();

private:
	int ChunkyNoise ( ivec2 offset );
	FastNoise::SmartNode<> fnGenerator;
	float scaleFactor = 0.01f;
	void PrepareDisplayVector ();
};

class TextBufferManager {
public:
	uvec2 displaySize;
	std::vector < Layer > layers;
	roguelikeGameState rgd;

	TextBufferManager ( uvec2 screenDimensions );
	~TextBufferManager ();

	void Populate ();
	void Update ();
	void DrawAllLayers ();

	GLint offsetUniformLocation;
	GLint displayUniformLocation;
	GLint alphaUniformLocation;
};

#endif
