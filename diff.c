#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGC_ERROR 1
#define BUFLEN 256
#define MAXSTRINGS 1024
#define GREEN "\x1B[1;32m"
#define RED "\x1B[1;31m"
#define NORMAL "\x1B[0m"


int vflag = 0, qflag = 0, iflag = 0, sflag = 0, yflag = 0,
    cflag = 0, uflag = 0, lcflag = 0, sclflag = 0, normal = 0;

typedef struct para para;
struct para {
  char** base;
  int filesize;
  int start;
  int stop;
  char* firstline;
  char* secondline;
};

FILE* openfile(const char* filename, const char* openflags);
char* yesorno(int condition);
para* para_make(char* base[], int size, int start, int stop);
para* para_first(char* base[], int size);
void  para_destroy(para* p);
para* para_next(para* p);
size_t para_filesize(para* p);
size_t para_size(para* p);
char** para_base(para* p);
char* para_info(para* p);
int   para_equal(para* p, para* q);
void para_print(para* p, void (*fp)(const char*));
void printleft(const char* left);
void printright(const char* right);
void printboth(const char* left_right);

para* para_make(char* base[], int filesize, int start, int stop) {
  para* p = (para*) malloc(sizeof(para));
  p->base = base;
  p->filesize = filesize;
  p->start = start;
  p->stop = stop;
  p->firstline = (p == NULL || start < 0) ? NULL : p->base[start];
  p->secondline = (p == NULL || start < 0 || filesize < 2) ? NULL : p->base[start + 1];
  return p;
}

para* para_first(char* base[], int size) {
  para* p = para_make(base, size, 0, -1);
  return para_next(p);
}

void para_destroy(para* p) { free(p); }

para* para_next(para* p) {
  if (p->stop == p->filesize) { return NULL; }

  int i;
  para* pnew = para_make(p->base, p->filesize, p->stop + 1, p->stop + 1);
  for (i = pnew->start; i < p->filesize && strcmp(p->base[i], "\n") != 0; ++i) { }
  pnew->stop = i;

  return pnew;
}
size_t para_filesize(para* p) { return p == NULL ? 0 : (size_t) (p->filesize); }

size_t para_size(para* p) { return p == NULL || p->stop < p->start ? 0 : (size_t) (p->stop - p->start + 1); }

char** para_base(para* p) { return p->base; }

char* para_info(para* p) {
  static char buf[BUFLEN];   // static for a reason
  snprintf(buf, sizeof(buf), "base: %p, filesize: %d, start: %d, stop: %d\n",
                  (void *)p->base, p->filesize, p->start, p->stop);
  return buf;  // buf MUST be static
}

int para_equal(para* p, para* q) {
  if (p == NULL || q == NULL) { return 0; }
  if (para_size(p) != para_size(q)) { return 0; }
  int i = p->start, j = q->start, equal = 0;
  while (i != para_filesize(p) && j != para_filesize(q) && (equal = strcmp(p->base[i], q->base[j])) == 0) { ++i; ++j; }
  return equal;
}

void para_print(para* p, void (*fp)(const char*)) {
  if (p == NULL) { return; }
  for (int i = p->start; i <= p->stop && i != p->filesize; ++i) { fp(p->base[i]); }
}

char* yesorno(int condition) { return condition == 0 ? "NO" : "YES"; }

FILE* openfile(const char* filename, const char* openflags) {
  FILE* f;
  if ((f = fopen(filename, openflags)) == NULL) {  printf("can't open '%s'\n", filename);  exit(1); }
  return f;
}

void printleft(const char* left) {
  char buf[BUFLEN];

  strcpy(buf, left);
  int j = 0, len = (int)strlen(buf) - 1;
  for (j = 0; j <= 48 - len ; ++j) { buf[len + j] = ' '; }
  buf[len + j++] = '<';
  buf[len + j++] = '\0';
  printf(GREEN"%s\n"NORMAL, buf);
}

void printright(const char* right) {
  if (right == NULL) { return; }
  printf(RED"%50s %s"NORMAL, ">", right);
}

void printboth(const char* left_right) {
  char buf[BUFLEN];
  size_t len = strlen(left_right);
  if (len > 0) { strncpy(buf, left_right, len); }
  buf[len - 1] = '\0';
  printf(NORMAL"%-50s %s", buf, left_right); }

void version(){ printf("Written by Luc Dang\n"); }

void setoptions(const char* arg, const char* s, const char* t, int* value){
    if(strcmp(arg, s) == 0 || (t != NULL && strcmp(arg, t) == 0)){ *value = 1;}
}

void init_options_files(int argc, const char* argv[]){
  while(argc-- > 0){
    const char* arg = *argv;
    setoptions(arg, "-v", "--version", &vflag);
    setoptions(arg, "-v", "--version", &vflag);
    setoptions(arg, "-q", "--brief", &qflag);
    setoptions(arg, "-i", "--ignore-case", &iflag);
    setoptions(arg, "-s", "--report-identical-files", &sflag);
    setoptions(arg, "-y", "--side-by-side", &yflag);
    setoptions(arg, "-c", "--context", &cflag);
    setoptions(arg, "-u", "--unified", &uflag);
    setoptions(arg, "--left-column", NULL, &lcflag);
    setoptions(arg, "--supress-common-lines", NULL, &sclflag);
    setoptions(arg, "--normal", NULL, &normal);
    argv++;
  }

}

void side_by_side(para* p, para* q){
  while(p != NULL){
    while(q != NULL && !para_equal(p, q)){
      para_print(q, printright);
      q = para_next(q);
    }
    while(p != NULL && q != NULL && para_equal(p,q)){
      para_print(p, printboth);
      p = para_next(p);
      q = para_next(q);
    }
    if( p!= NULL){
      para_print(p, printleft);
      p = para_next(p);
    }
  }
  while(q != NULL){
    para_print(q, printright);
    q = para_next(q);
  }
}

int is_different(para* p, para* q){
  if(para_filesize(p) != para_filesize(q)){
    return 1;
  }
  while(p != NULL && q != NULL){
    if(para_equal(p, q) != 0){
      return 1;
    }
    p = para_next(p);
    q = para_next(q);
  }
  return 0;
}

int main(int argc, const char *argv[]) {
  char buf[BUFLEN];
  char *strings1[MAXSTRINGS], *strings2[MAXSTRINGS];

  memset(buf, 0, sizeof(buf));
  memset(strings1, 0, sizeof(strings1));
  memset(strings2, 0, sizeof(strings2));

  if (argc < 3) { fprintf(stderr, "Usage: ./diff file1 file2\n");  exit(ARGC_ERROR); }
  if (argc == 3){ normal = 1; }

  init_options_files(--argc, argv++);

  if(vflag){ version(); return 0; }
  if((cflag && uflag) || (yflag && uflag) || (yflag && cflag)){ printf("Conflicting output styles\n"); return 1; }

  FILE *fin1 = openfile(argv[argc-2], "r");
  FILE *fin2 = openfile(argv[argc-1], "r");

  int count1 = 0, count2 = 0;
  while (!feof(fin1) && fgets(buf, BUFLEN, fin1) != NULL) { strings1[count1++] = strdup(buf); }
  while (!feof(fin2) && fgets(buf, BUFLEN, fin2) != NULL) { strings2[count2++] = strdup(buf); }

  para* p = para_first(strings1, count1);
  para* q = para_first(strings2, count2);


  if(qflag && !sflag){
    if(is_different(p,q)){ printf("Files %s and %s differ\n", argv[argc-2], argv[argc-1]); }
    return 0;
  }
  if(sflag || (sflag && qflag)){
    if(!is_different(p,q)){ printf("Files %s and %s are identical\n", argv[argc-2], argv[argc-1]); return 0;}
  }
  side_by_side(p,q);
  //
  // printf("p is: %s", para_info(p));
  // printf("q is: %s", para_info(q));
  // para_print(p, printleft);
  // para_print(q, printright);
  // printf("p and q are equal: %s\n\n", (para_equal(p, q)) == 0 ? "YES" : "NO");




  // printf("\nTODO: check line by line in a paragraph, using '|' for differences");
  // printf("\nTODO: this starter code does not yet handle printing all of fin1's paragraphs.");
  // printf("\nTODO: handle the rest of diff's options\n");
  // printf("\tAs Tolkien said it, '...and miles to go before I sleep.'\n\n");
  fclose(fin1);
  fclose(fin2);
  return 0;
}