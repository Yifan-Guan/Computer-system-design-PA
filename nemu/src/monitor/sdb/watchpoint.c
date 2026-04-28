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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  char expr[256];
  sword_t value;
  bool enable;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  if (free_ == NULL) {
    panic("No free watchpoint!\n");
  }

  WP *p = free_;
  free_ = free_->next;

  p->next = head;
  head = p;

  return p;
}

void free_wp(WP *wp) {
  if (wp == NULL) {
    panic("Invalid watchpoint!\n");
  }

  WP *p = head;
  WP *prev = NULL;
  while (p) {
    if (p == wp) {
      if (prev) {
        prev->next = p->next;
      } else {
        head = p->next;
      }
      p->next = free_;
      free_ = p;
      return;
    }
    prev = p;
    p = p->next;
  }

  printf("Watchpoint not found!\n");
}

void insert_wp(char *e) {
  WP *wp = new_wp();
  strncpy(wp->expr, e, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';
  Assert(strlen(e) < sizeof(wp->expr), "Expression is too long for watchpoint!");

  bool success = false;
  wp->value = expr(e, &success);
  if (!success) {
    printf("Failed to evaluate expression: %s\n", e);
    free_wp(wp);
    return;
  }
  wp->enable = true;
}

void delete_wp(int no) {
  WP *p = head;
  while (p) {
    if (p->NO == no) {
      free_wp(p);
      return;
    }
    p = p->next;
  }
  printf("Watchpoint %d not found!\n", no);
}

void check_wp() {
  WP *p = head;
  while (p) {
    bool success = false;
    sword_t new_value = expr(p->expr, &success);
    if (success) {
      if (new_value != p->value) {
        nemu_state.state = NEMU_STOP;
        printf("Watchpoint %d triggered: %s changed from 0x%08x to 0x%08x\n",
               p->NO, p->expr, p->value, new_value);
        p->value = new_value;
      }
    } else {
      printf("Failed to evaluate watchpoint %d: %s\n", p->NO, p->expr);
    }
    p = p->next;
  }
}

void wp_display() {
  WP *p = head;
  printf("NO \t Type \t Enable \t Expression \t Value \n");
  while (p && p != free_) {
    printf("%d \t %s \t %s \t %s \t 0x%08x\n", p->NO, "watchpoint", p->enable ? "yes" : "no", p->expr, p->value);
    p = p->next;
  }
}

