#version 430 core

// font data
layout( binding = 0 ) uniform sampler2D currentFont;
// const ivec2 characterAtlasDimensions = ivec2( 1, 256 );
const ivec2 characterAtlasDimensions = ivec2( 16, 16 );

// data buffer - characters to be rendered
layout( binding = 1, rgba8ui ) uniform uimage2D dataTexture;

uniform vec2 resolution;
uniform ivec2 numChars;
uniform ivec2 offset;

out vec4 fragmentOutput;
void main() {

	fragmentOutput = vec4( 0.0, 0.0, 0.0, 0.0 );

	ivec2 characterIndex = ivec2( floor( ( gl_FragCoord.xy / resolution ) * numChars ) );
	characterIndex.y = numChars.y - 1 - characterIndex.y;

	// bounds check before texture read
	const ivec2 bounds = imageSize( dataTexture ).xy;
	ivec2 readLocation = characterIndex.xy - offset;
	uvec4 characterID = uvec4( 0 );
	bool boundsCheck = all( greaterThanEqual( readLocation.xy, ivec2( 0 ) ) ) && all( lessThan( readLocation.xy, bounds ) );
	// will hold per-character color in .rgb, character identifier in .a
	if ( boundsCheck ) {
		characterID = imageLoad( dataTexture, readLocation );
	}

	// uv for referencing atlas
	vec2 positionOnCharacter = fract( ( gl_FragCoord.xy / resolution ) * numChars );
	positionOnCharacter.y = 1.0 - positionOnCharacter.y;

	// texture reference of the given character, at computed uv
	if ( characterID.a != 0 && boundsCheck ) {
		vec2 atlasReadLocation =  vec2( ( float( characterID.a % characterAtlasDimensions.y ) + positionOnCharacter.x ) / characterAtlasDimensions.x, ( float( characterID.a / characterAtlasDimensions.x ) + positionOnCharacter.y ) / characterAtlasDimensions.y );
		fragmentOutput = texture( currentFont, atlasReadLocation );
	}

	// use the color held in the first three channels
	fragmentOutput.xyz *= ( characterID.rgb / 255.0 );

	// int idx = characterIndex.x + characterIndex.y * numChars.x;
	// fragmentOutput = texture( currentFont, vec2(  positionOnCharacter.x / characterAtlasDimensions.x, ( float( idx ) + positionOnCharacter.y ) / characterAtlasDimensions.y ) );
}
