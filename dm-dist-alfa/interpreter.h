/* ************************************************************************
*  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
************************************************************************* */

void command_interpreter(struct char_data *ch, char *argument);
int search_block(char *arg, char **list, bool exact);
int old_search_block(char *argument,int begin,int length,char **list,int mode);
char lower( char c );
void argument_interpreter(char *argument, char *first_arg, char *second_arg);
char *one_argument(char *argument,char *first_arg);
int fill_word(char *argument);
void half_chop(char *string, char *arg1, char *arg2);
void nanny(struct descriptor_data *d, char *arg);
int is_abbrev(char *arg1, char *arg2);


struct command_info
{
	void (*command_pointer) (struct char_data *ch, char *argument, int cmd);
	byte minimum_position;
	byte minimum_level;
};
