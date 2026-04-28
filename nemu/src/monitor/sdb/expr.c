/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NUM, TK_PARE_L, TK_PARE_R,
  TK_NEG,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"\\(", TK_PARE_L},   // left parenthesis
  {"\\)", TK_PARE_R},   // right parenthesis
  {"[0-9]+", TK_NUM},   // decimal number
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[1000] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        Assert(substr_len < 32, "tokens is overflowed");

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_NUM: {
            assert(substr_len < 32);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            tokens[nr_token].type = TK_NUM;
            nr_token ++;
            break;
          }
          case '+': {
            tokens[nr_token].type = '+';
            nr_token ++;
            break;
          }
          case '-': {
            if (nr_token == 0 ||
                tokens[nr_token - 1].type == TK_PARE_L ||
                tokens[nr_token - 1].type == '+' ||
                tokens[nr_token - 1].type == '-' ||
                tokens[nr_token - 1].type == '*' ||
                tokens[nr_token - 1].type == '/' ||
                tokens[nr_token - 1].type == TK_EQ) {
              tokens[nr_token].type = TK_NEG;
            } else {
              tokens[nr_token].type = '-';
            }
            nr_token ++;
            break;
          }
          case '*': {
            tokens[nr_token].type = '*';
            nr_token ++;
            break;
          }
          case '/': {
            tokens[nr_token].type = '/';
            nr_token ++;
            break;
          }
          case TK_PARE_L: {
            tokens[nr_token].type = TK_PARE_L;
            nr_token ++;
            break;
          }
          case TK_PARE_R: {
            tokens[nr_token].type = TK_PARE_R;
            nr_token ++;
            break;
          }
          case TK_EQ: {
            tokens[nr_token].type = TK_EQ;
            nr_token ++;
            break;
          }
          default: assert(0);
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q, bool* need_split) {
  if (tokens[p].type != TK_PARE_L || tokens[q].type != TK_PARE_R) {
    return false;
  }

  int cnt = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_PARE_L) {
      cnt++;
    }
    else if (tokens[i].type == TK_PARE_R) {
      cnt--;
      if (cnt < 0) {
        return false;
      }
      if (cnt == 0 && i != q) {
        *need_split = true;
      }
    }
  }

  return cnt == 0;
}

sword_t eval(int p, int q, bool *success) {
  bool need_split = false;
  if (p > q) {
    /* Bad expression */
    *success = false;
    Assert(0, "Bad expression, %d > %d", p, q);
    return 0;
  }
  else if (p == q) {
    /* Single token. Just return the value of it. */
    if (tokens[p].type == TK_NUM) {
      return atoi(tokens[p].str);
    }
    else {
      *success = false;
      Assert(0, "Invalid token");
      return 0;
    }
  }
  else if (check_parentheses(p, q, &need_split) == true && !need_split) {
    /* The expression is surrounded by a. matched pair of parentheses. */
    return eval(p + 1, q - 1, success);
  }
  else {
    /* We should do more things here. */
    int main_op = -1;
    int main_op_precedence = 0x7fffffff;
    int parentheses_cnt = 0;
    for (int i = p; i <= q; i++) {
      if (tokens[i].type == TK_PARE_L) {
        parentheses_cnt++;
      }
      else if (tokens[i].type == TK_PARE_R) {
        parentheses_cnt--;
      }
      else if (parentheses_cnt == 0) {
        int precedence = 0x7fffffff;
        switch (tokens[i].type) {
          case TK_EQ: precedence = 0; break;
          case '+':
          case '-': precedence = 1; break;
          case '*':
          case '/': precedence = 2; break;
          case TK_NEG: precedence = 3; break;
          default: break;
        }
        if (precedence <= main_op_precedence) {
          main_op_precedence = precedence;
          main_op = i;
        }
      }
    }

    if (main_op == -1) {
      /* No operator found. This should not happen. */
      *success = false;
      Assert(0, "No operator found");
      return 0;
    }

    if (tokens[main_op].type == TK_NEG) {
      sword_t val = eval(main_op + 1, q, success);
      if (*success == false) {
        Assert(0, "Failed to evaluate the operand of unary minus");
        return 0;
      }
      return -val;
    } 

    sword_t val1 = eval(p, main_op - 1, success);
    if (*success == false) {
      Assert(0, "Failed to evaluate the first operand");
      return 0;
    }
    sword_t val2 = eval(main_op + 1, q, success);
    if (*success == false) {
      Assert(0, "Failed to evaluate the second operand");
      return 0;
    }

    printf("main_op: %s, val1: %d, val2: %d\n", tokens[main_op].type == TK_EQ ? "==" : (char[]){tokens[main_op].type, '\0'}, val1, val2);

    switch (tokens[main_op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': {
        if (val2 == 0) {
          *success = false;
          Assert(0, "Division by zero");
          return 0;
        }
        return val1 / val2;
      }
      case TK_EQ: return val1 == val2;
      default: assert(0);
    }
  }
}

sword_t expr(char *e, bool *success) {
  *success = true;

  if (!make_token(e)) {
    *success = false;
    Assert(0, "make_token failed");
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  sword_t result = eval(0, nr_token - 1, success);

  return result;
}

void test_expr() {
  FILE *fp = fopen("/tmp/expression_test", "r");
  assert(fp != NULL);
  
  char buf[65536];

  while (fgets(buf, sizeof(buf), fp) != NULL) {
    char *pos = strchr(buf, '\n');
    if (pos) {
      *pos = '\0';
    }
    char *result_str = strtok(buf, " ");
    char *expr_str = strtok(NULL, "");
    assert(expr_str != NULL && result_str != NULL);

    printf("%s\t", expr_str);
    printf("%s\t", result_str);

    bool success = true;
    sword_t result = expr(expr_str, &success);
    if (success) {
      printf("result: %d\t", result);
      if (result == atoi(result_str)) {
        printf("correct\n");
      }
      else {
        printf("wrong\n");
      }
    }
    else {
      printf("invalid expression\n");
    }
  }
  fclose(fp);
}
