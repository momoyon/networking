#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <raylib.h>

typedef struct Connection Connection;

struct Connection {
	Vector2 *from;
	Vector2 *to;
};

void draw_connection(Connection *c, bool debug);

#endif // _CONNECTION_H_
