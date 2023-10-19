#include <string.h>
#include <assert.h>
#include <stdio.h>

struct Rule {
  int len;
  int* tokens;// i < 0 means key
};

struct Def {
  const char* name;
  int len;
  Rule* rules;
};

struct Grammar {
  int len;
  Def* defs; // -i indexes
};

#include "gcache.h"

bool is_nonterminal(int key) {
  return key <= 0;
}

max_count_t rule_get_num_strings(int key, int rule, int pos, int* tokens, int len, Grammar* grammar, int l_str);

max_count_t key_get_num_strings(int key, Grammar* grammar, int l_str) {
  if (!is_nonterminal(key)) {
    return l_str == 1 ? 1 : 0; // we assume every terminal is size 1
  } 

  max_count_t v = get_key_count_at_length(key, l_str);
  if (v != UNINITIALIZED) return v;

  max_count_t s = 0;
  Def* def = &grammar->defs[-key]; // indexed negative
  for (int i = 0; i < def->len; i++) {
    Rule* r = &def->rules[i];
    s += rule_get_num_strings(key, i, 0, r->tokens, r->len, grammar, l_str);
  }
  set_key_count_at_length(key, l_str, s);
  return s;
}

max_count_t rule_get_num_strings(int key, int rule, int pos, int* tokens, int len, Grammar* grammar, int l_str) {
  max_count_t v = get_rule_count_at_length(key, rule, pos, l_str);
  if (v != UNINITIALIZED) return v;

  if (!len) {
    v = 0;
    set_rule_count_at_length(key, rule, pos, l_str, v);
    return v; // empty rule
  }
  if (len == 1) {
    v = key_get_num_strings(tokens[0], grammar, l_str); // if not tail
    set_rule_count_at_length(key, rule, pos, l_str, v);
    return v;
  }

  max_count_t sum_rule = 0;
  int token = tokens[0];
  int *tail = tokens+1;
  for(int l_str_x = 1; l_str_x < l_str+1; l_str_x++) {
    max_count_t s_ = key_get_num_strings(token, grammar, l_str_x);
    if (s_ == 0) continue;

    max_count_t rem = rule_get_num_strings(key, rule, pos+1, tail, len-1, grammar, l_str - l_str_x );
    sum_rule += s_ * rem;
  }
  set_rule_count_at_length(key, rule, pos, l_str, sum_rule);
  return sum_rule;
}

void rule_get_num_strings_at(int key, int rule, int pos, int* tokens, int len, Grammar* grammar, int l_str, max_count_t at);

void key_get_num_strings_at(int key, Grammar* grammar, int l_str, max_count_t at) {
  if (!is_nonterminal(key)) {
    assert(at == 0);
    printf("%c", key);
    return; // we assume every terminal is size 1
  }

  max_count_t s = 0;
  Def* def = &grammar->defs[-key]; // indexed negative
  for (int i = 0; i < def->len; i++) {
    Rule* r = &def->rules[i];
    max_count_t v = rule_get_num_strings(key, i, 0, r->tokens, r->len, grammar, l_str);
    if ((s + v) > at) {
      //printf("reached\n");
      // at - s is the remainder of at that we need to reach.
      rule_get_num_strings_at(key, i, 0, r->tokens, r->len, grammar, l_str, at - s);
      break;
    }
    s += v;
  }
}

max_count_t find_largest_s(max_count_t sum_rule, max_count_t s_, max_count_t rem, max_count_t at) {
  // find the smallest s_ so that sum_rule + (s_*rem) >= at
  assert(sum_rule + (s_*rem) >= at);
  max_count_t small_s = 0;
  if ((sum_rule + (small_s*rem)) >= at) {
      return small_s;
  }
  for (;sum_rule + (small_s*rem) < at; small_s++);
  small_s --;
  assert(sum_rule + (small_s*rem) < at);
  return small_s;
}

void rule_get_num_strings_at(int key, int rule, int pos, int* tokens, int len, Grammar* grammar, int l_str, max_count_t at) {
  if (!len) return; // empty rule
  if (len == 1) {
    max_count_t v = key_get_num_strings(tokens[0], grammar, l_str); // if not tail
    if (v > at) { // if v == 0, we should not be going in.
      key_get_num_strings_at(tokens[0], grammar, l_str, at); // if not tail
    }
    return;
  }

  max_count_t sum_rule = 0;
  int token = tokens[0];
  int *tail = tokens+1;
  for(int l_str_x = 1; l_str_x < l_str+1; l_str_x++) {
    max_count_t s_ = key_get_num_strings(token, grammar, l_str_x);
    if (s_ == 0) continue;

    max_count_t rem = rule_get_num_strings(key, rule, pos+1, tail, len-1, grammar, l_str - l_str_x );
    max_count_t sr = s_ * rem;

    if ((sum_rule + sr) > at) {
      // At this point, we know that we have overshot the `at`, which means that
      // our `at` is somewhere in the current rule. So, dig in.
      // There is a complication here. We overshot using the
      // head (s_) and tail (rem) combinations. Note that we could have
      // multiple heads (0..si..s_), any of which could have lead to `at`
      //
      // We need to split here correctly. Which is the largest 0..si_ so that
      // sum_rule + (si_ * rem) < at and sum_rule + ((si_+1)*rem) >=at ?
      // This would be the head the remainder would be the tail.
      max_count_t smallest_si_ = find_largest_s(sum_rule, s_, rem, at);
      key_get_num_strings_at(token, grammar, l_str_x, smallest_si_);

      max_count_t r = at - (sum_rule + smallest_si_*rem);

      // Now this remainder should go into rule at.
      rule_get_num_strings_at(key, rule, pos+1, tail, len-1, grammar, l_str - l_str_x, r);
      // We don't need to continue.
      return;
    }
    sum_rule += sr;
  }
  return;
}


#include "gdef.h"
void count_rules_at(int l_str, int sample) {
  max_count_t count = key_get_num_strings(0, &g_grammar, l_str);
  printf("%lu\n", count);
  if (sample > -1) {
    if (sample >= count) {
       printf("inaccessible (%d/%lu)\n", sample, count);
       return;
    }
    key_get_num_strings_at(0, &g_grammar, l_str, sample);
    return;
  }
  for (int i = 0; i < count; i++) {
    if (i > count) break;
    key_get_num_strings_at(0, &g_grammar, l_str, i); // this is l_str long
    printf("\n");
  }
}

int
main(int argc, char* argv[]) {
  int depth = 10;
  int sample = -1;
  if (argc > 1) {
     depth = atoi(argv[1]);
  }
  if (argc > 2) {
     sample = atoi(argv[2]);
  }
  count_rules_at(depth, sample);
  printf("\n");
  return 0;
}
