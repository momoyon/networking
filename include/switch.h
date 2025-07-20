#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <predecls.h>
#include <config.h>

#define COMMONLIB_REMOVE_PREFIX
#include <commonlib.h>

typedef struct Switch_console Switch_console;
typedef struct Console_line Console_line;
typedef struct Console_lines Console_lines;

struct Console_line {
	char buff[CONSOLE_LINE_BUFF_CAP];
    size_t count;
};

struct Console_lines {
	Console_line *items;
	size_t count;
	size_t capacity;
}; // @darr


struct Switch_console {
	Console_lines lines;
	int cursor; // offset in the line
	int line;   // line number
};

typedef struct {
	int vlan;
	Nic *nic;
	int nic_id; // NOTE: Only used when loading switches.
} Port;

typedef struct Switch Switch;
struct Switch {
	Switch_console console;
	Port fa[1][4];
};

void make_switch(Switch *switch_out, Arena *arena);
void make_switch_console(Switch_console *console_out, Arena *arena);
bool input_to_console(Switch_console *console);
float get_cursor_offset(Switch_console *console);

#endif // _SWITCH_H_
