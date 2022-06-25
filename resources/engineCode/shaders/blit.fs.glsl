#version 430 core

// font data
layout( binding = 0 ) uniform sampler2D currentFont;
const ivec2 characterAtlasDimensions = ivec2( 1, 256 );

// data buffer - characters to be rendered
layout( binding = 1, rgba8ui ) uniform uimage2D dataTexture;


uniform vec2 resolution;
uniform ivec2 numChars;

out vec4 fragmentOutput;
void main() {


	// fragmentOutput = texture( current, gl_FragCoord.xy / resolution );
	fragmentOutput = vec4( 0.0, 0.0, 0.0, 1.0 );

	ivec2 characterIndex = ivec2( floor( ( gl_FragCoord.xy / resolution ) * numChars ) );
	characterIndex.y = numChars.y - 1 - characterIndex.y;

	// will hold per-character color in .rgb, character identifier in .a
	uint characterID = imageLoad( dataTexture, characterIndex.xy ).a;

	// uv for referencing atlas
	vec2 positionOnCharacter = fract( ( gl_FragCoord.xy / resolution ) * numChars );
	positionOnCharacter.y = 1.0 - positionOnCharacter.y;

	// list out characters - replace currentChar with characterID to use the data from the dataTexture
	float currentChar = characterIndex.x + characterIndex.y * numChars.x;
	// fragmentOutput.xyz = texture( currentFont, vec2( positionOnCharacter.x / characterAtlasDimensions.x, ( currentChar + positionOnCharacter.y ) / characterAtlasDimensions.y ) ).xyz;
	fragmentOutput.xyz = texture( currentFont, vec2( positionOnCharacter.x / characterAtlasDimensions.x, ( float( characterID ) + positionOnCharacter.y ) / characterAtlasDimensions.y ) ).xyz;



	// checkerboard
	if ( int( currentChar ) % 2 == 0 && fragmentOutput.xyz == vec3( 0.0 ) )
		fragmentOutput.xyz = vec3( 0.618 );


	// fragmentOutput.xy = positionOnCharacter;
}
