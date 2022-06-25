#version 430 core

// font data
layout( binding = 0 ) uniform sampler2D currentFont;
uniform ivec2 characterAtlasDimensions;


uniform vec2 resolution;
uniform ivec2 numChars;

out vec4 fragmentOutput;
void main() {


	// fragmentOutput = texture( current, gl_FragCoord.xy / resolution );
	fragmentOutput = vec4( 0.0, 0.0, 0.0, 1.0 );

	ivec2 characterIndex = ivec2( floor( ( gl_FragCoord.xy / resolution ) * numChars ) );
	vec2 positionOnCharacter = fract( ( gl_FragCoord.xy / resolution ) * numChars );

	fragmentOutput.xy = positionOnCharacter;
	fragmentOutput.z = sin( characterIndex.x );
}
