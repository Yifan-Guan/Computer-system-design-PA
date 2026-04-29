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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

vaddr_t vaddr_read(vaddr_t addr, int len);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_si(char *args) {
  int steps = 1;

  if (args != NULL) {
    steps = atoi(args);
  }

  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) { return 0; }

  if (strcmp(args, "r") == 0) {
    printf("registers:\n");
    isa_reg_display();
  }
  else if (strcmp(args, "w") == 0) {
    printf("watchpoints:\n");
    wp_display();
  } else if (strcmp(args, "rb") == 0) {
    ring_buf_print();
  } else if (strcmp(args, "mb") == 0) {
    mtrace_print();
  } else {
    printf("Unknown info command '%s'\n", args);
    printf("Usage: info r - show registers\n");
    printf("       info w - show watchpoints\n");
    printf("       info rb - show the instruction ring buffer\n");
    printf("       info mb - show the memory trace buffer\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) { return 0; }

  char *arg1 = strtok(args, " ");
  char *arg2 = strtok(NULL, "");

  if (arg1 == NULL || arg2 == NULL) {
    printf("Usage: x N EXPR\n");
    return 0;
  }

  int n = atoi(arg1);
  // vaddr_t addr = expr(arg2);
  vaddr_t addr = strtol(arg2, NULL, 0);

  for (int i = 0; i < n; i++) {
    if (i % 4 == 0) {
      printf("\n 0x%08x: \t", addr + i * 4);
    }
    printf("%08x \t", vaddr_read(addr + i * 4, 4));
  }
  printf("\n");

  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_watch(char *args) {
  if (args == NULL) {
    printf("Usage: watch EXPR\n");
    return 0;
  }
  insert_wp(args);
  return 0;
}

static int cmd_rm_watch(char *args) {
  if (args == NULL) {
    printf("Usage: delete WP_NO\n");
    return 0;
  }
  int no = atoi(args);
  delete_wp(no);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step into instruction", cmd_si },
  { "info", "Display registers by 'r' or watchpoints by 'w'", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "watch", "Set a watchpoint for an expression", cmd_watch },
  { "d", "Delete a watchpoint by its number", cmd_rm_watch },

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
