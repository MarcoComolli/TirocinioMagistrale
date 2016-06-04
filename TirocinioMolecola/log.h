#ifndef LOG_H
#define LOG_H


void debug(const char* st);
void debug(const char* st, int i);
void debug(const char* st, int i, int j);
void debug(const char* st, int i, int j, int k);
void debug(const char* st, const char* st2);
void debug(const char* st, float i, float j, float k);

void critical(const char* st);

#endif // LOG_H
