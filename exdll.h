#ifndef _EXDLL_H_
#define _EXDLL_H_

#include <windows.h>
#include <stdio.h>

#if defined(__GNUC__)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#ifdef _countof
#define COUNTOF _countof
#else
#define COUNTOF(a) (sizeof(a)/sizeof(a[0]))
#endif

#ifndef NSISCALL
#  define NSISCALL __stdcall
#endif

// only include this file from one place in your DLL.
// (it is all static, if you use it in two places it will fail)

#define EXDLL_INIT()           {  \
        g_stringsize=string_size; \
        g_stacktop=stacktop;      \
        g_variables=variables; }

// For page showing plug-ins
#define WM_NOTIFY_OUTER_NEXT (WM_USER+0x8)
#define WM_NOTIFY_CUSTOM_READY (WM_USER+0xd)
#define NOTIFY_BYE_BYE 'x'

typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;


unsigned int g_stringsize;
stack_t **g_stacktop;
char *g_variables;

int NSISCALL popstring(char *str); // 0 on success, 1 on empty stack
int NSISCALL popstringn(char *str, int maxlen); // with length limit, pass 0 for g_stringsize
int NSISCALL popint(); // pops an integer
int NSISCALL popint_or(); // with support for or'ing (2|4|8)
int NSISCALL myatoi(const char *s); // converts a string to an integer
unsigned NSISCALL myatou(const char *s); // converts a string to an unsigned integer, decimal only
int NSISCALL myatoi_or(const char *s); // with support for or'ing (2|4|8)
void NSISCALL pushstring(const char *str);
void NSISCALL pushint(int value);
char * NSISCALL getuservariable(const int varnum);
void NSISCALL setuservariable(const int varnum, const char *var);

enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
INST_LANG,      // $LANGUAGE
__INST_LAST
};

typedef struct {
  int autoclose;
  int all_user_var;
  int exec_error;
  int abort;
  int exec_reboot;
  int reboot_called;
  int XXX_cur_insttype; // deprecated
  int XXX_insttype_changed; // deprecated
  int silent;
  int instdir_error;
  int rtl;
  int errlvl;
  int alter_reg_view;
  int status_update;
} exec_flags_type;

typedef struct {
  exec_flags_type *exec_flags;
  int (*ExecuteCodeSegment)(int, HWND);
  void (*validate_filename)(char *);
} extra_parameters;

// utility functions (not required but often useful)
int NSISCALL popstring(char *str)
{
  stack_t *th;
  if (!g_stacktop || !*g_stacktop) 
	  return 1;

  th=(*g_stacktop);
  lstrcpyA(str,th->text);
  *g_stacktop = th->next;

  GlobalFree((HGLOBAL)th);
  return 0;
}

int NSISCALL popstringn(char *str, int maxlen)
{
  stack_t *th;
  if (!g_stacktop || !*g_stacktop) 
	  return 1;

  th=(*g_stacktop);
  if (str) 
	  strncpy(str,th->text,maxlen?maxlen:g_stringsize);

  *g_stacktop = th->next;

  GlobalFree((HGLOBAL)th);
  return 0;
}

void NSISCALL pushstring(const char *str)
{
  stack_t *th;
  if (!g_stacktop)
	  return;

  th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
  lstrcpynA(th->text,str,g_stringsize);
  th->next=*g_stacktop;
  *g_stacktop=th;
}

 char *  NSISCALL getuservariable(const int varnum)
{
  if (varnum < 0 || varnum >= __INST_LAST) 
	  return NULL;

  return g_variables+varnum*g_stringsize;
}

void NSISCALL setuservariable(const int varnum, const char *var)
{
	if (var != NULL && varnum >= 0 && varnum < __INST_LAST) 
		lstrcpyA(g_variables + varnum*g_stringsize, var);
}


// playing with integers

int NSISCALL myatoi(const char *s)
{
  int v=0;
  if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
  {
    s++;
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '9') 
		  c-='0';
      else if (c >= 'a' && c <= 'f') 
		  c-='a'-10;
      else if (c >= 'A' && c <= 'F') 
		  c-='A'-10;
      else 
		  break;
      v<<=4;
      v+=c;
    }
  }
  else if (*s == '0' && s[1] <= '7' && s[1] >= '0')
  {
    for (;;)
    {
      int c=*(++s);
      if (c >= '0' && c <= '7') 
		  c-='0';
      else 
		  break;
      v<<=3;
      v+=c;
    }
  }
  else
  {
    int sign=0;
    if (*s == '-')
		sign++; 
	else 
		s--;
    for (;;)
    {
      int c=*(++s) - '0';
      if (c < 0 || c > 9)
		  break;
      v*=10;
      v+=c;
    }
    if (sign) v = -v;
  }

  return v;
}

unsigned NSISCALL myatou(const char *s)
{
  unsigned int v=0;

  for (;;)
  {
    unsigned int c=*s++;

    if (c >= '0' && c <= '9') 
		c-='0';
    else 
		break;
    v*=10;
    v+=c;
  }
  return v;
}

int NSISCALL myatoi_or(const char *s)
{
  int v=0;
  if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
  {
    s++;
    for (;;)
    {
      int c=*(++s);

      if (c >= '0' && c <= '9')
		  c-='0';
      else if (c >= 'a' && c <= 'f')
		  c-='a'-10;
      else if (c >= 'A' && c <= 'F') 
		  c-='A'-10;
      else break;
      v<<=4;
      v+=c;
    }
  }
  else if (*s == '0' && s[1] <= '7' && s[1] >= '0')
  {
    for (;;)
    {
      int c=*(++s);

      if (c >= '0' && c <= '7') 
		  c-='0';
      else 
		  break;
      v<<=3;
      v+=c;
    }
  }
  else
  {
    int sign=0;
    if (*s == '-') 
		sign++; 
	else 
		s--;
    for (;;)
    {
      int c=*(++s) - '0';
      if (c < 0 || c > 9) 
		  break;
      v*=10;
      v+=c;
    }
    if (sign) v = -v;
  }

  // Support for simple ORed expressions
  if (*s == '|') 
  {
      v |= myatoi_or(s+1);
  }

  return v;
}

int NSISCALL popint()
{
  char buf[128];
  if (popstringn(buf,COUNTOF(buf)))
    return 0;

  return myatoi(buf);
}

int NSISCALL popint_or()
{
  char buf[128];
  if (popstringn(buf,COUNTOF(buf)))
    return 0;

  return myatoi_or(buf);
}

void NSISCALL pushint(int value)
{
	char buffer[1024];
	printf(buffer, "%d", value);
	pushstring(buffer);
}

#endif//_EXDLL_H_
