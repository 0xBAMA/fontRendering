#include "includes.h"

class textBuffer {
public:
	textBuffer( int x, int y ) : dimensions( x, y ) {
		buffer.resize( x * y );
	}

	void Update() {

	}

	glm::ivec2 dimensions;

private:
	std::vector< unsigned char > buffer;
};
