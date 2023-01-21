//
#include "t3ch_config.h"
//
int t3ch_log(const char *text, va_list args);
int t3ch_log_gen_new(int lastId);
int t3ch_log_gen_old(int fromPos);
void t3ch_log_get(char *out);
