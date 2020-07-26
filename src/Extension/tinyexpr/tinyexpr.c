/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015, 2016 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "tinyexpr.h"
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef NAN
#define NAN (0.0/0.0)
#endif

typedef const double (*te_fun2)(double, double);

enum {
    TOK_NULL = TE_CLOSURE7+1, TOK_ERROR, TOK_END, TOK_SEP,
    TOK_OPEN, TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_INFIX
};


enum {TE_CONSTANT = 1};


typedef struct state {
    const char *start;
    const char *next;
    int type;
    union {double value; const double *bound; const void *function;};
    void *context;

    const te_variable *lookup;
    int lookup_len;
} state;


#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

#define IS_PURE(TYPE) (((TYPE) & TE_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE) & TE_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE) (((TYPE) & TE_CLOSURE0) != 0)
#define ARITY(TYPE) ( ((TYPE) & (TE_FUNCTION0 | TE_CLOSURE0)) ? ((TYPE) & 0x00000007) : 0 )
#define NEW_EXPR(type, ...) new_expr((type), (const te_expr*[]){__VA_ARGS__})

static te_expr *new_expr(const int type, const te_expr *parameters[]) {
    const int arity = ARITY(type);
    const int psize = sizeof(void*) * arity;
    const int size = sizeof(te_expr) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
    te_expr *ret = malloc(size);
    memset(ret, 0, size);
    if (arity && parameters) {
        memcpy(ret->parameters, parameters, psize);
    }
    ret->type = type;
    ret->bound = 0;
    return ret;
}


void te_free_parameters(te_expr *n) {
    if (!n) return;
    switch (TYPE_MASK(n->type)) {
        case TE_FUNCTION7: case TE_CLOSURE7: te_free(n->parameters[6]);
        case TE_FUNCTION6: case TE_CLOSURE6: te_free(n->parameters[5]);
        case TE_FUNCTION5: case TE_CLOSURE5: te_free(n->parameters[4]);
        case TE_FUNCTION4: case TE_CLOSURE4: te_free(n->parameters[3]);
        case TE_FUNCTION3: case TE_CLOSURE3: te_free(n->parameters[2]);
        case TE_FUNCTION2: case TE_CLOSURE2: te_free(n->parameters[1]);
        case TE_FUNCTION1: case TE_CLOSURE1: te_free(n->parameters[0]);
    }
}


void te_free(te_expr *n) {
    if (!n) return;
    te_free_parameters(n);
    free(n);
}


static double pi() {return 3.14159265358979323846;}
static double e() {return 2.71828182845904523536;}
static double __not(double a) { return ~(int)a; }

#pragma function (ceil)
#pragma function (floor)

static const te_variable functions[] = {
    /* must be in alphabetical order */
    {"abs", fabs,     TE_FUNCTION1 | TE_FLAG_PURE},
    {"acos", acos,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"asin", asin,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"atan", atan,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"atan2", atan2,  TE_FUNCTION2 | TE_FLAG_PURE},
    {"ceil", ceil,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"cos", cos,      TE_FUNCTION1 | TE_FLAG_PURE},
    {"cosh", cosh,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"e", e,          TE_FUNCTION0 | TE_FLAG_PURE},
    {"exp", exp,      TE_FUNCTION1 | TE_FLAG_PURE},
    {"floor", floor,  TE_FUNCTION1 | TE_FLAG_PURE},
    {"ln", log,       TE_FUNCTION1 | TE_FLAG_PURE},
    {"log", log10,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"not", __not,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"pi", pi,        TE_FUNCTION0 | TE_FLAG_PURE},
    {"pow", pow,      TE_FUNCTION2 | TE_FLAG_PURE},
    {"sin", sin,      TE_FUNCTION1 | TE_FLAG_PURE},
    {"sinh", sinh,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"sqrt", sqrt,    TE_FUNCTION1 | TE_FLAG_PURE},
    {"tan", tan,      TE_FUNCTION1 | TE_FLAG_PURE},
    {"tanh", tanh,    TE_FUNCTION1 | TE_FLAG_PURE},
    {0}
};

static const te_variable *find_builtin(const char *name, int len) {
    int imin = 0;
    int imax = sizeof(functions) / sizeof(te_variable) - 2;

    /*Binary search.*/
    while (imax >= imin) {
        const int i = (imin + ((imax-imin)/2));
        int c = strncmp(name, functions[i].name, len);
        if (!c) c = '\0' - functions[i].name[len];
        if (c == 0) {
            return functions + i;
        } else if (c > 0) {
            imin = i + 1;
        } else {
            imax = i - 1;
        }
    }

    return 0;
}

static const te_variable *find_lookup(const state *s, const char *name, int len) {
    int i;
    if (!s->lookup) return 0;
    for (i = 0; i < s->lookup_len; ++i) {
        if (strncmp(name, s->lookup[i].name, len) == 0 && s->lookup[i].name[len] == '\0') {
            return s->lookup + i;
        }
    }
    return 0;
}



static double add(double a, double b) {return a + b;}
static double sub(double a, double b) {return a - b;}
static double mul(double a, double b) {return a * b;}
static double divide(double a, double b) {return a / b;}
static double negate(double a) {return -a;}
static double comma(double a, double b) {return b;}

static double __div(double a, double b) {
  if ((int)b == 0)
  {
    return NAN;
  }
  return (int)(a / b);
}
static double __mod(double a, double b) {
  if ((int)b == 0)
  {
    return NAN;
  }
  return (int)a % (int)b;
}
static double __shl(double a, double b) { return (int)a << (int)b; }
static double __shr(double a, double b) { return (int)a >> (int)b; }
static double __and(double a, double b) { return (int)a & (int)b; }
static double __or(double a, double b) { return (int)a | (int)b; }
static double __xor(double a, double b) { return (int)a ^ (int)b; }

static const te_operator operators[] = {
  { "div", __div },
  { "mod", __mod },
  { "shl", __shl },
  { "shr", __shr },
  { "and", __and },
  { "or", __or },
  { "xor", __xor }
};

static const te_operator *find_operator(const char *name, int len)
{
  int i;
  for (i = 0; i < _countof(operators); ++i)
  {
    if (strncmp(name, operators[i].name, len) == 0 && operators[i].name[len] == '\0')
    {
      return operators + i;
    }
  }
  return 0;
}

static const te_operator *find_operator_by_function(const void* address)
{
  int i;
  for (i = 0; i < _countof(operators); ++i)
  {
    if (operators[i].address == address)
    {
      return operators + i;
    }
  }
  return 0;
}

int is_operation(const char ch)
{
  switch (ch)
  {
  case '+':
  case '-':
  case '*':
  case '/':
  case '^':
  case '%':
  case '(':
  case ')':
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    return 1;
  default:
    return 0;
  }
}

int is_hex_prefix(const char* pStr)
{
  return ((pStr[0] == '0') && (pStr[1] == 'x'));
}

int is_radix_postfix(const char ch)
{
  switch (ch)
  {
  case 'b':
  case 'o':
  case 'h':
  case 'd':
    return 1;
  default:
    return 0;
  }
}

int is_number_by_radix(const char** pStr, const int radixTest, double* pValue, int* radixRes)
{
  char* next = (char*)*pStr;
  double resDouble = 0;
  long int res = 0;
  if (radixTest == 10)
  {
    if (!is_hex_prefix(*pStr))
    {
      if ((*pStr[0] >= '0' && *pStr[0] <= '9') || *pStr[0] == '.')
      {
        resDouble = strtod(*pStr, &next);
      }
    }
    if ((next == *pStr) || (resDouble == HUGE_VAL))
    {
      return 0;
    }
  }
  else
  {
    res = strtol(*pStr, &next, radixTest);
    if ((res == LONG_MIN) || (res == LONG_MAX) || (next[0] == '.'))
    {
      return 0;
    }
  }
  *pStr = next;
  if (pValue)
  {
    *pValue = (resDouble != 0) ? resDouble : res;
  }
  if (radixRes)
  {
    *radixRes = radixTest;
  }
  return 1;
}

int get_alphanum_length(const char *str)
{
  if (str)
  {
    const char *start = str;
    while (isalnum((unsigned char)str[0]))
    {
      ++str;
    }
    return str - start;
  }
  return 0;
}

int is_number(const char** pStr, double* pValue)
{
  if (is_operation(*pStr[0])
      || find_operator(*pStr, get_alphanum_length(*pStr))
      || find_builtin(*pStr, get_alphanum_length(*pStr)))
  {
    return 0;
  }

  int res = 0;
  const char* start = *pStr;
  int radix = 0;
  char radixPrefix = (is_hex_prefix(start) == 1) ? 'h' : 0;
  if (is_number_by_radix(&start, 16, pValue, &radix)
      || is_number_by_radix(&start, 10, pValue, &radix))
  {
    char radixPostfix = (radix == 16) ? 'h' : 'd';
    if ((radix == 16) && !is_radix_postfix(start[0]))
    {
      if (radixPrefix == 0)
      {
        radixPostfix = 'd';
        --start;
      }
    }
    if (is_radix_postfix(start[0]))
    {
      radixPostfix = start[0];
    }
    switch (radixPostfix)
    {
      case 'b':
        res = is_number_by_radix(pStr, 2, pValue, NULL);
        break;
      case 'o':
        res = is_number_by_radix(pStr, 8, pValue, NULL);
        break;
      case 'h':
        if (radix == 16)
        {
          *pStr = start;
          res = 1;
        }
        else
        {
          res = is_number_by_radix(pStr, 16, pValue, NULL);
        }
        break;
      case 'd':
        if (radix == 10)
        {
          *pStr = start;
          res = 1;
        }
        else
        {
          res = is_number_by_radix(pStr, 10, pValue, NULL);
        }
        break;
      default:
        res = is_number_by_radix(pStr, 10, pValue, NULL);
        break;
    }
    if (is_radix_postfix((*pStr)[0]))
    {
      ++*pStr;
    }
  }
  return res;
}

void next_token(state *s) {
    s->type = TOK_NULL;

    if (!*s->next){
        s->type = TOK_END;
        return;
    }

    do {

        /* Try reading a number. */
        double value = 0;
        if (is_number(&s->next, &value) != 0) {
            s->value = value;
            s->type = TOK_NUMBER;
        } else {
            /* Look for a variable or builtin function call. */
            if (s->next[0] >= 'a' && s->next[0] <= 'z') {
                const char *start;
                start = s->next;
                while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9')) s->next++;

                const te_operator *operator = find_operator(start, s->next - start);
                if (operator)
                {
                  s->type = TOK_INFIX;
                  s->function = operator->address;
                  break;
                }

                const te_variable *var = find_lookup(s, start, s->next - start);
                if (!var) var = find_builtin(start, s->next - start);

                if (!var) {
                    s->type = TOK_ERROR;
                } else {
                    switch(TYPE_MASK(var->type))
                    {
                        case TE_VARIABLE:
                            s->type = TOK_VARIABLE;
                            s->bound = var->address;
                            break;

                        case TE_CLOSURE0: case TE_CLOSURE1: case TE_CLOSURE2: case TE_CLOSURE3:
                        case TE_CLOSURE4: case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
                            s->context = var->context;

                        case TE_FUNCTION0: case TE_FUNCTION1: case TE_FUNCTION2: case TE_FUNCTION3:
                        case TE_FUNCTION4: case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
                            s->type = var->type;
                            s->function = var->address;
                            break;
                    }
                }

            } else {
                /* Look for an operator or special character. */
                switch (s->next++[0]) {
                    case '+': s->type = TOK_INFIX; s->function = add; break;
                    case '-': s->type = TOK_INFIX; s->function = sub; break;
                    case '*': s->type = TOK_INFIX; s->function = mul; break;
                    case '/': s->type = TOK_INFIX; s->function = divide; break;
                    case '^': s->type = TOK_INFIX; s->function = pow; break;
                    case '%': s->type = TOK_INFIX; s->function = fmod; break;
                    case '(': s->type = TOK_OPEN; break;
                    case ')': s->type = TOK_CLOSE; break;
                    //case ',': s->type = TOK_SEP; break;
                    case ' ': case '\t': case '\n': case '\r': break;
                    default: s->type = TOK_ERROR; break;
                }
            }
        }
    } while (s->type == TOK_NULL);
}


static te_expr *list(state *s);
static te_expr *expr(state *s);
static te_expr *power(state *s);

static te_expr *base(state *s) {
    /* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
    te_expr *ret;
    int arity;

    switch (TYPE_MASK(s->type)) {
        case TOK_NUMBER:
            ret = new_expr(TE_CONSTANT, 0);
            ret->value = s->value;
            next_token(s);
            break;

        case TOK_VARIABLE:
            ret = new_expr(TE_VARIABLE, 0);
            ret->bound = s->bound;
            next_token(s);
            break;

        case TE_FUNCTION0:
        case TE_CLOSURE0:
            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[0] = s->context;
            next_token(s);
            if (s->type == TOK_OPEN) {
                next_token(s);
                if (s->type != TOK_CLOSE) {
                    s->type = TOK_ERROR;
                } else {
                    next_token(s);
                }
            }
            break;

        case TE_FUNCTION1:
        case TE_CLOSURE1:
            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[1] = s->context;
            next_token(s);
            ret->parameters[0] = power(s);
            break;

        case TE_FUNCTION2: case TE_FUNCTION3: case TE_FUNCTION4:
        case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
        case TE_CLOSURE2: case TE_CLOSURE3: case TE_CLOSURE4:
        case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
            arity = ARITY(s->type);

            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[arity] = s->context;
            next_token(s);

            if (s->type != TOK_OPEN) {
                s->type = TOK_ERROR;
            } else {
                int i;
                for(i = 0; i < arity; i++) {
                    next_token(s);
                    ret->parameters[i] = expr(s);
                    if(s->type != TOK_SEP) {
                        break;
                    }
                }
                if(s->type != TOK_CLOSE || i != arity - 1) {
                    s->type = TOK_ERROR;
                } else {
                    next_token(s);
                }
            }

            break;

        case TOK_OPEN:
            next_token(s);
            ret = list(s);
            if (s->type != TOK_CLOSE) {
                s->type = TOK_ERROR;
            } else {
                next_token(s);
            }
            break;

        default:
            ret = new_expr(0, 0);
            s->type = TOK_ERROR;
            ret->value = NAN;
            break;
    }

    return ret;
}


static te_expr *power(state *s) {
    /* <power>     =    {("-" | "+")} <base> */
    int sign = 1;
    while (s->type == TOK_INFIX && (s->function == add || s->function == sub)) {
        if (s->function == sub) sign = -sign;
        next_token(s);
    }

    te_expr *ret;

    if (sign == 1) {
        ret = base(s);
    }
    else {
        ret = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
        ret->function = negate;
    }

    return ret;
}


static te_expr *factor(state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_expr *ret = power(s);

    while (s->type == TOK_INFIX && (s->function == pow)) {
        te_fun2 t = s->function;
        next_token(s);
        ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
        ret->function = t;
    }

    return ret;
}


static te_expr *term(state *s) {
    /* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
    te_expr *ret = factor(s);

    while (s->type == TOK_INFIX
        && (s->function == mul || s->function == divide || s->function == fmod
            || find_operator_by_function(s->function))) {
        te_fun2 t = s->function;
        next_token(s);
        ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, factor(s));
        ret->function = t;
    }

    return ret;
}


static te_expr *expr(state *s) {
    /* <expr>      =    <term> {("+" | "-") <term>} */
    te_expr *ret = term(s);

    while (s->type == TOK_INFIX && (s->function == add || s->function == sub)) {
        te_fun2 t = s->function;
        next_token(s);
        ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, term(s));
        ret->function = t;
    }

    return ret;
}


static te_expr *list(state *s) {
    /* <list>      =    <expr> {"," <expr>} */
    te_expr *ret = expr(s);

    while (s->type == TOK_SEP) {
        next_token(s);
        ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, expr(s));
        ret->function = comma;
    }

    return ret;
}


#define TE_FUN(...) ((double(*)(__VA_ARGS__))n->function)
#define M(e) te_eval(n->parameters[e])


double te_eval(const te_expr *n) {
    if (!n) return NAN;

    switch(TYPE_MASK(n->type)) {
        case TE_CONSTANT: return n->value;
        case TE_VARIABLE: return *n->bound;

        case TE_FUNCTION0: case TE_FUNCTION1: case TE_FUNCTION2: case TE_FUNCTION3:
        case TE_FUNCTION4: case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
            switch(ARITY(n->type)) {
                case 0: return TE_FUN(void)();
                case 1: return TE_FUN(double)(M(0));
                case 2: return TE_FUN(double, double)(M(0), M(1));
                case 3: return TE_FUN(double, double, double)(M(0), M(1), M(2));
                case 4: return TE_FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
                case 5: return TE_FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
                case 6: return TE_FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return TE_FUN(double, double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        case TE_CLOSURE0: case TE_CLOSURE1: case TE_CLOSURE2: case TE_CLOSURE3:
        case TE_CLOSURE4: case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
            switch(ARITY(n->type)) {
                case 0: return TE_FUN(void*)(n->parameters[0]);
                case 1: return TE_FUN(void*, double)(n->parameters[1], M(0));
                case 2: return TE_FUN(void*, double, double)(n->parameters[2], M(0), M(1));
                case 3: return TE_FUN(void*, double, double, double)(n->parameters[3], M(0), M(1), M(2));
                case 4: return TE_FUN(void*, double, double, double, double)(n->parameters[4], M(0), M(1), M(2), M(3));
                case 5: return TE_FUN(void*, double, double, double, double, double)(n->parameters[5], M(0), M(1), M(2), M(3), M(4));
                case 6: return TE_FUN(void*, double, double, double, double, double, double)(n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return TE_FUN(void*, double, double, double, double, double, double, double)(n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        default: return NAN;
    }

}

#undef TE_FUN
#undef M

static void optimize(te_expr *n) {
    /* Evaluates as much as possible. */
    if (n->type == TE_CONSTANT) return;
    if (n->type == TE_VARIABLE) return;

    /* Only optimize out functions flagged as pure. */
    if (IS_PURE(n->type)) {
        const int arity = ARITY(n->type);
        int known = 1;
        int i;
        for (i = 0; i < arity; ++i) {
            optimize(n->parameters[i]);
            if (((te_expr*)(n->parameters[i]))->type != TE_CONSTANT) {
                known = 0;
            }
        }
        if (known) {
            const double value = te_eval(n);
            te_free_parameters(n);
            n->type = TE_CONSTANT;
            n->value = value;
        }
    }
}


te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, int *error) {
    state s;
    s.start = s.next = expression;
    s.lookup = variables;
    s.lookup_len = var_count;

    next_token(&s);
    te_expr *root = list(&s);

    if (s.type != TOK_END) {
        te_free(root);
        if (error) {
            *error = (s.next - s.start);
            if (*error == 0) *error = 1;
        }
        return 0;
    } else {
        optimize(root);
        if (error) *error = 0;
        return root;
    }
}


double te_interp(const char *expression, int *error) {
    te_expr *n = te_compile(expression, 0, 0, error);
    double ret;
    if (n) {
        ret = te_eval(n);
        te_free(n);
    } else {
        ret = NAN;
    }
    return ret;
}

static void pn (const te_expr *n, int depth) {
    int i, arity;
    printf("%*s", depth, "");

    switch(TYPE_MASK(n->type)) {
    case TE_CONSTANT: printf("%f\n", n->value); break;
    case TE_VARIABLE: printf("bound %p\n", n->bound); break;

    case TE_FUNCTION0: case TE_FUNCTION1: case TE_FUNCTION2: case TE_FUNCTION3:
    case TE_FUNCTION4: case TE_FUNCTION5: case TE_FUNCTION6: case TE_FUNCTION7:
    case TE_CLOSURE0: case TE_CLOSURE1: case TE_CLOSURE2: case TE_CLOSURE3:
    case TE_CLOSURE4: case TE_CLOSURE5: case TE_CLOSURE6: case TE_CLOSURE7:
         arity = ARITY(n->type);
         printf("f%d", arity);
         for(i = 0; i < arity; i++) {
             printf(" %p", n->parameters[i]);
         }
         printf("\n");
         for(i = 0; i < arity; i++) {
             pn(n->parameters[i], depth + 1);
         }
         break;
    }
}


void te_print(const te_expr *n) {
    pn(n, 0);
}

int is_digit_or_dot(const unsigned char ch)
{
  return (isxdigit(ch)
          || (ch == '.')
          || (ch == 'b')
          || (ch == 'd')
          || (ch == 'o')
          || (ch == 'h')
          || (ch == 'x')
          || (ch == 'X')) ? 1 : 0;
}

int is_newline(const unsigned char ch)
{
  return (ch == '\r') || (ch == '\n') ? 1 : 0;
}

int is_space_or_newline(const unsigned char ch)
{
  return (isspace(ch) || is_newline(ch)) ? 1 : 0;
}

char *te_trimwhitespace(unsigned char *str)
{
  // Trim leading space
  while (isspace((const unsigned char)*str)) str++;

  if (*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  unsigned char *end = str + strlen(str) - 1;
  while (end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end + 1) = 0;

  return str;
}

unsigned char *remove_chars(unsigned char *pszSrc, const unsigned char *pszCharsToSkip)
{
  unsigned char* pszRes = malloc(strlen(pszSrc) + 1);
  unsigned char *pszCurrent = pszRes;
  unsigned char *pszTest = pszSrc;
  while (*pszTest)
  {
    if (!strchr(pszCharsToSkip, *pszTest))
    {
      *pszCurrent++ = *pszTest;
    }
    ++pszTest;
  }
  *pszCurrent++ = 0x0;
  return pszRes;
}

unsigned char *replace_char(unsigned char *pszSrc, const unsigned char chFrom, const unsigned char chTo)
{
  unsigned char *pszTest = pszSrc;
  while (*pszTest)
  {
    if (*pszTest == chFrom)
    {
      *pszTest = chTo;
    }
    ++pszTest;
  }
  return pszSrc;
}

// convert to a single line, insert + between numbers
unsigned char *apply_digits_and_dots(unsigned char *pszSrc)
{
  unsigned char *pszTest = pszSrc;
  char prevChar = 0x0;
  while (*pszTest)
  {
    if (is_space_or_newline(*pszTest))
    {
      if (!is_space_or_newline(prevChar))
      {
        prevChar = *pszTest;
        *pszTest = '+';
      }
      else
      {
        prevChar = *pszTest;
        *pszTest = ' ';
      }
    }
    else
    {
      prevChar = *pszTest;
    }
    ++pszTest;
  }
  return pszSrc;
}

char *te_prepare(unsigned char *pszSrc)
{
  unsigned char *pszEqualsPos = strchr(pszSrc, '=');
  if (pszEqualsPos)
  {
    *pszEqualsPos = 0x0;
  }
  pszSrc = te_trimwhitespace(pszSrc);

  // convert ',' to '.' if possible
  if (!strchr(pszSrc, '.'))
  {
    unsigned char *pszTest = replace_char(remove_chars(pszSrc, "$"), ',', '.');
    double exprValue = 0.0;
    if (is_valid_expression(pszTest, 0, &exprValue))
    {
      strcpy_s(pszSrc, strlen(pszTest) + 1, pszTest);
    }
    free(pszTest);
  }

  int onlyDigitsAndDots = 1;
  unsigned char *pszTest = remove_chars(pszSrc, "$,");
  strcpy_s(pszSrc, strlen(pszTest) + 1, pszTest);
  free(pszTest);

  pszTest = pszSrc;
  while (*pszTest)
  {
    if (onlyDigitsAndDots)
    {
      onlyDigitsAndDots &= (is_digit_or_dot(*pszTest) || is_space_or_newline(*pszTest));
    }
    ++pszTest;
  }

  if (onlyDigitsAndDots)
  {
    pszSrc = apply_digits_and_dots(pszSrc);
  }

  return pszSrc;
}

int is_valid_expression(unsigned char *pszText, const int bUsePrepare, double *pValue)
{
  if (!pValue)
  {
    return 0;
  }
  int error = 0;
  if (bUsePrepare)
  {
    pszText = te_prepare(pszText);
  }
  *pValue = te_interp(pszText, &error);
  return ((error == 0) && !isnan(*pValue) && !isinf(*pValue)) ? 1 : 0;
}
