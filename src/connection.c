#include <connection.h>

void draw_connection(Connection *c, bool debug) {
	(void)debug;

	if (c->from && c->to)
		DrawLineBezier(*c->from, *c->to, 1, WHITE);
}
