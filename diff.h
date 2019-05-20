#ifndef diff_h
#define diff_h

#include <stdio.h>

typedef struct para para;
struct para {
  char** base;
  char* firstline;
  char* secondline;
  int filesize;
  int start;
  int stop;
};

para* para_make(char* base[], int filesize, int start, int stop);
para* para_first(char* base[], int size);
void para_destroy(para* p);
para* para_next(para* p);
size_t para_filesize(para* p);
size_t para_size(para* p);
char** para_base(para* p);
char* para_info(para* p);
int strcmp_ignore(char* s, char* t);
int para_equal(para* p, para* q);
int para_compare(para* p, para* q);
int is_different(para* p, para* q);
void para_print(para* p, void (*fp)(const char*));
FILE* openfile(const char* filename, const char* openflags);
void printleft(const char* left);
void printleftcolumn(const char* left);
void para_printcu(para* p);
void printright(const char* right);
//print both logic courtesy of Professor McCarthy
void para_printboth(para* p, para* q, void (*fp)(const char*, const char*));
void printbothhelper(const char* left, const char* right, int leftparen, int nocommon, char symbol);
void printnocommon(const char* left, const char* right);
void printleftparen(const char* left, const char* right);
void printboth(const char* left, const char* right);
void para_printnormal(para* p, const char* character);
void version(void);
void setoptions(const char* arg, const char* s, const char* t, int* value);
void init_options(int argc, const char* argv[]);
void side_by_side(para* p, para* q);
void diff_normal(para* p, para* q);
void diff_context(para* p, para* q);
void diff_unified(para* p, para* q);
void filestats(const char* file1, const char* file2);

#endif
