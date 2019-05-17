#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include "diff.h"

#define ARGC_ERROR 1
#define BUFLEN 256
#define MAXSTRINGS 1024


static int vflag = 0, qflag = 0, iflag = 0, sflag = 0, yflag = 0,
    cflag = 0, uflag = 0, lcflag = 0, sclflag = 0, normal = 0, num = 3;

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

int strcmp_ignore(char* s, char* t){
  while(*s != '\0' && (*s == *t)){
    s++; t++;
  }
  return tolower(*s) - tolower(*t);
}

int para_equal(para* p, para* q) {
  if (p == NULL || q == NULL) { return 0; }
  if (para_size(p) != para_size(q)) { return 0; }
  if (p->start >= p->filesize || q->start >= q->filesize) { return 0; }
  int i = p->start, j = q->start, equal = 0;
  while (i != p->stop && j!= q->stop && (equal = strcmp(p->base[i], q->base[j])) == 0) { ++i; ++j; }
  return 1;
}

int para_compare(para* p, para* q){
  int i = p->start, j = q->start, compare = 0;
  while(i != p->stop && j != q->stop){
    compare += iflag ? strcmp_ignore(p->base[i], q->base[j]) : strcmp(p->base[i], q->base[j]);
    ++i; ++j;
  }
  return compare;
}

int is_different(para* p, para* q){
  if(para_filesize(p) != para_filesize(q)){
    return 1;
  }
  while(p != NULL && q != NULL){
    if(para_compare(p, q) != 0){
      return 1;
    }
    p = para_next(p);
    q = para_next(q);
  }
  return 0;
}

void para_print(para* p, void (*fp)(const char*)) {
  if (p == NULL) { return; }
  for (int i = p->start; i <= p->stop && i != p->filesize; ++i) { fp(p->base[i]); }
}

FILE* openfile(const char* filename, const char* openflags) {
  FILE* f;
  if ((f = fopen(filename, openflags)) == NULL) {  printf("can't open '%s'\n", filename);  exit(1); }
  return f;
}

void printleft(const char* left) {
  char buf[BUFLEN];

  strcpy(buf, left);
  int j = 0, len = (int)strlen(buf) - 1;
  if(len > 61) { len = 61; }
  for (j = 0; j <= 61 - len ; ++j) { buf[len + j] = ' '; }
  buf[len + j++] = '<';
  buf[len + j++] = '\0';
  printf("%s\n", buf);
}

void printleftcolumn(const char* left){
  char buf[BUFLEN];

  strcpy(buf, left);
  int j = 0, len = (int)strlen(buf) - 1;
  for (j = 0; j <= 61 - len ; ++j) { buf[len + j] = ' '; }
  buf[len + j++] = '(';
  buf[len + j++] = '\0';
  printf("%s\n", buf);
}

void para_printcu(para* p) {
  if (p == NULL) { return; }
  for (int i = p->start; i < p->start + num && i != p->filesize; ++i) {
    char buf[BUFLEN];

    strcpy(buf, p->base[i]);
    int j = 0, len = (int)strlen(buf) - 1;
    for (j = 0; j <= 61 - len ; ++j) { buf[len + j] = ' '; }
    buf[len + j++] = '\0';
      printf("%s\n", buf);

  }
}

void printright(const char* right) {
  if (right == NULL) { return; }
  printf("%63s %s", ">", right);
}

//print both logic courtesy of Professor McCarthy
void para_printboth(para* p, para* q, void (*fp)(const char*, const char*)) {
  if (p == NULL || q == NULL) { return; }
  for (int i = p->start, j = q->start; i <= p->stop && i != p->filesize
                                    && j <= q->stop && j != q->filesize; ++i, ++j) {
    fp(p->base[i], q->base[j]);
  }
}
void printbothhelper(const char* left, const char* right, int leftparen, int nocommon, char symbol){
  char buf[BUFLEN];
  symbol = ((strcmp(left, right) == 0) ? symbol : '|');
  size_t len = strlen(left);
  if(len > 0) { strncpy(buf, left, len); }
  buf[len - 1] = '\0';

  if(symbol != '|' && nocommon == 1) { return; }

  printf("%-61s %c ", buf, symbol);
  if(symbol == '|'){
    printf("%s", right);
  }
  else{
    printf("%s", (leftparen ? "\n" : right));
  }
}
void printnocommon(const char* left, const char* right) { printbothhelper(left, right, 0,1, ' '); }
void printleftparen(const char* left, const char* right) { printbothhelper(left, right, 1,0, '('); }
void printboth(const char* left, const char* right) { printbothhelper(left, right, 0,0, ' '); }

void para_printnormal(para* p, const char* character){
  if (p == NULL) { return; }
  for (int i = p->start; i <= p->stop && i != p->filesize; ++i) {
    printf("%s %s", character, p->base[i]);
  }
}

void version(){
  printf("diff (CSUF diffutils) 1.0.0\n");
  printf("Copyright (C) 2019 CSUF\n");
  printf("This program comes with NO WARRANTY, to the extent permitted by law.\n");
  printf("You may redistribute copies of this program\n");
  printf("under the terms of the GNU General Public License.\n");
  printf("For more information about these matters, see the file named COPYING.\n");
  printf("\nWritten by Luc Dang\n");
}

void setoptions(const char* arg, const char* s, const char* t, int* value){
    if(strcmp(arg, s) == 0 || (t != NULL && strcmp(arg, t) == 0)){ *value = 1;}
}

void init_options(int argc, const char* argv[]){
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
    setoptions(arg, "--suppress-common-lines", NULL, &sclflag);
    setoptions(arg, "--normal", NULL, &normal);
    argv++;
  }

}

void side_by_side(para* p, para* q){
  int foundmatch = 0;
  para* qlast = q;
  while(p != NULL){
    qlast = q;
    foundmatch = 0;
    while(q != NULL && (foundmatch = para_equal(p, q)) == 0){
      q = para_next(q);
    }
    q = qlast;

    if(foundmatch){
      while((foundmatch = para_equal(p, q)) == 0){
        para_print(q, printright);
        q = para_next(q);
        qlast = q;
      }
      if(sclflag){ para_printboth(p, q, printnocommon); }
      else if(lcflag){ para_printboth(p, q, printleftparen); }
      else { para_printboth(p, q, printboth);  }
      p = para_next(p);
      q = para_next(q);
    }
    else{
      para_print(p, printleft);
      p = para_next(p);
    }
  }
  while(q != NULL){
    para_print(q, printright);
    q = para_next(q);
  }
}

void diff_normal(para* p, para* q){
  int foundmatch = 0;
  para* qlast = q;
  while(p != NULL){
    qlast = q;
    foundmatch = 0;
    while(q != NULL && (foundmatch = para_equal(p, q)) == 0){
      q = para_next(q);
    }
    q = qlast;

    if(foundmatch){
      while((foundmatch = para_equal(p, q)) == 0){
        printf("%da", p->start);
        printf("%d,%d\n", q->start + 1, q->stop + 1);
        para_printnormal(q, ">");
        q = para_next(q);
        qlast = q;
      }
      p = para_next(p);
      q = para_next(q);
    }
    else{
      printf("%d,%d", p->start + 1, p->stop + 1);
      printf("d%d\n", q->start);
      para_printnormal(p, "<");
      p = para_next(p);
    }
  }
  while(q != NULL){
    //printf("%d,%d\n", q->start + 1, q->stop + 1);
    //printf("---\n");
    para_printnormal(q, ">");
    q = para_next(q);
  }
}

void diff_context(para* p, para* q){
  int foundmatch = 0;
  para* qlast = q;
  while(p != NULL){
    qlast = q;
    foundmatch = 0;
    while(q != NULL && (foundmatch = para_equal(p, q)) == 0){
      q = para_next(q);
    }
    q = qlast;

    if(foundmatch){
      while((foundmatch = para_equal(p, q)) == 0){
        printf("%da", p->start);
        printf("%d,%d\n", q->start + 1, q->stop + 1);
        para_printnormal(q, "+");
        q = para_next(q);
        qlast = q;
      }
      para_printcu(p);
      p = para_next(p);
      q = para_next(q);
    }
    else{
      printf("%d,%d", p->start + 1, p->stop + 1);
      printf("d%d\n", q->start);
      para_printnormal(p, "-");
      p = para_next(p);
    }
  }
  while(q != NULL){
    printf("---\n");
    printf("%d,%d\n", q->start + 1, q->stop + 1);
    para_printnormal(q, "+");
    q = para_next(q);
  }
}

void diff_unified(para* p, para* q){
  int foundmatch = 0;
  para* qlast = q;
  while(p != NULL){
    qlast = q;
    foundmatch = 0;
    while(q != NULL && (foundmatch = para_equal(p, q)) == 0){
      q = para_next(q);
    }
    q = qlast;

    if(foundmatch){
      while((foundmatch = para_equal(p, q)) == 0){
        printf("%da", p->start);
        printf("%d,%d\n", q->start + 1, q->stop + 1);
        para_printnormal(q, "+");
        q = para_next(q);
        qlast = q;
      }
      para_printcu(p);
      p = para_next(p);
      q = para_next(q);
    }
    else{
      printf("%d,%d", p->start + 1, p->stop + 1);
      printf("d%d\n", q->start);
      para_printnormal(p, "-");
      p = para_next(p);
    }
  }
  while(q != NULL){
    printf("---\n");
    printf("%d,%d\n", q->start + 1, q->stop + 1);
    para_printnormal(q, "+");
    q = para_next(q);
  }
}

void filestats(const char* file1, const char* file2){
  struct stat filestat1;
  struct stat filestat2;
  stat(file1, &filestat1);
  stat(file2, &filestat2);
  printf("%s %s\t%s", cflag ? "***" : "---" , file1, ctime(&filestat1.st_mtime));
  printf("%s %s\t%s", cflag ? "---" : "+++" , file2, ctime(&filestat2.st_mtime));
}

int main(int argc, const char *argv[]) {
  char buf[BUFLEN];
  char *strings1[MAXSTRINGS], *strings2[MAXSTRINGS];

  memset(buf, 0, sizeof(buf));
  memset(strings1, 0, sizeof(strings1));
  memset(strings2, 0, sizeof(strings2));

  if (argc < 3) { fprintf(stderr, "Usage: ./diff file1 file2\n");  exit(ARGC_ERROR); }
  if (argc == 3 || (strcmp(argv[1], "--normal") == 0) || (!cflag && !uflag && !yflag && !lcflag)){ normal = 1; }

  init_options(--argc, argv++);

  if(vflag){ version(); return 0; }
  if((cflag && uflag) || (yflag && uflag) || (yflag && cflag)){ printf("Conflicting output styles\n"); return 1; }

  FILE *fin1 = openfile(argv[argc-2], "r");
  FILE *fin2 = openfile(argv[argc-1], "r");

  int count1 = 0, count2 = 0;
  while (!feof(fin1) && fgets(buf, BUFLEN, fin1) != NULL) { strings1[count1++] = strdup(buf); }
  while (!feof(fin2) && fgets(buf, BUFLEN, fin2) != NULL) { strings2[count2++] = strdup(buf); }

  fclose(fin1);
  fclose(fin2);

  para* p = para_first(strings1, count1);
  para* q = para_first(strings2, count2);

  if(qflag || sflag){
    if(qflag){
      if(is_different(p,q)){ printf("Files %s and %s differ\n", argv[argc-2], argv[argc-1]); return 0;}
    }
    if(sflag){
      if(!is_different(p,q)){
        printf("Files %s and %s are identical\n", argv[argc-2], argv[argc-1]);
        return 0;
      }
    }
  }

  if(yflag){
    side_by_side(p,q);
    if(sflag && !is_different(p, q)){ printf("Files %s and %s are identical\n", argv[argc-2], argv[argc-1]); }
    return 0;
  }
  if(cflag){
    filestats(argv[argc-2], argv[argc- 1]);
    diff_context(p, q);
    return 0;
  }
  if(uflag){
    filestats(argv[argc-2], argv[argc- 1]);
    diff_unified(p, q);
    return 0;
  }
  diff_normal(p, q);

  // printf("\nTODO: check line by line in a paragraph, using '|' for differences");
  // printf("\nTODO: this starter code does not yet handle printing all of fin1's paragraphs.");
  // printf("\nTODO: handle the rest of diff's options\n");
  // printf("\tAs Tolkien said it, '...and miles to go before I sleep.'\n\n");
  return 0;
}
