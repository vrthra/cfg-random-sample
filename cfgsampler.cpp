#include <string.h>
#include <assert.h>
#include <stdio.h>

class TreeNode {
  public:
    int key; // greater than 256 indicate a nonterminal
    TreeNode* children[2];
};

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

// def is_nonterminal(key):
//     return (key[0], key[-1]) == ('<', '>')

bool is_nonterminal(int key) {
  return key <= 0;
}

// def modifiable(tree):
//     name, children, *rest = tree
//     if not is_nonterminal(name): return [name, []]
//     else:
//       return [name, [modifiable(c) for c in children]]
// 
// def tree_to_string(tree_):
//     tree = modifiable(tree_)
//     expanded = []
//     to_expand = [tree]
//     while to_expand:
//         (key, children, *rest), *to_expand = to_expand
//         if is_nonterminal(key):
//             #assert children # not necessary
//             to_expand = children + to_expand
//         else:
//             assert not children
//             expanded.append(key)
//     return ''.join(expanded)
int MAX_TREE_NODES_IN_STACK = 1000;
int MAX_STRING_SIZE=1000;

char* tree_to_string(TreeNode* tree, char* expanded) {
    TreeNode* to_expand[MAX_TREE_NODES_IN_STACK]; // Array to store nodes to expand
    int to_expand_size = 1;
    to_expand[0] = tree;
    to_expand[1] = 0; // remove

    int expanded_size = 0;

    while (to_expand_size > 0) {
        TreeNode* current = to_expand[--to_expand_size]; // Pop the last node

        if (is_nonterminal(current->key)) {
            if (current->children[1])
              to_expand[to_expand_size++] = current->children[1];
            if (current->children[0])
              to_expand[to_expand_size++] = current->children[0];
        } else {
            expanded[expanded_size++] = (char) current->key;
        }
    }
    expanded[expanded_size++] = '\0'; // Null-terminate the string
    return expanded;
}

void test_tree_to_string() {
  TreeNode left1 = { .key = '(', .children = 0 };
  TreeNode right1 = { .key = ')', .children = 0 };
  TreeNode rroot = { .key = -1, .children = {&left1, &right1} };
  TreeNode left2 = { .key = '(', .children = 0 };
  TreeNode right2 = { .key = ')', .children = 0 };
  TreeNode lroot = { .key = -1, .children = {&left2, &right2} };
  TreeNode root = { .key = -1, .children = {&rroot, &lroot} };

  char expanded[MAX_STRING_SIZE]; // Array to store expanded characters
  char* v = tree_to_string(&root, expanded);
  printf("%s\n", v);
}

// def key_get_num_strings(key, grammar, l_str):
//     if not is_nonterminal(key):
//         return 1 if l_str == len(key) else 0
//     s = 0
//     rules = grammar[key]
//     for rule in rules:
//         s += rule_get_num_strings(rule, grammar, l_str) 
//     return s

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

// def rule_get_num_strings(rule, grammar, l_str):
//     if not rule: return 0
// 
//     token, *tail = rule
//     if not tail:
//         return key_get_num_strings(token, grammar, l_str)
// 
//     sum_rule = 0
//     for l_str_x in range(1, l_str+1): # inclusive
//         s_ = key_get_num_strings(token, grammar, l_str_x)
//         if s_ == 0: continue
// 
//         rem = rule_get_num_strings(tail, grammar, l_str - l_str_x)
//         sum_rule += s_ * rem
//     return sum_rule

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

void test_count_rules() {
#include "gdef.h"
  // Make sure to set the max_l_str in build-grammar-cache.py appropriately
  int l_str = 8;
  max_count_t count = key_get_num_strings(0, &g, l_str);
  //l_str = 32;// I think this overflows
  max_count_t count2 = key_get_num_strings(0, &g, l_str);
  printf("%lu, %lu\n", count, count2);
}


void rule_get_num_strings_at(int key, int rule, int pos, int* tokens, int len, Grammar* grammar, int l_str, max_count_t at);

void key_get_num_strings_at(int key, Grammar* grammar, int l_str, max_count_t at) {
  if (!is_nonterminal(key)) { 
    printf("%c", key);
    return; // we assume every terminal is size 1
  }

  max_count_t s = 0;
  Def* def = &grammar->defs[-key]; // indexed negative
  for (int i = 0; i < def->len; i++) {
    Rule* r = &def->rules[i];
    max_count_t v = rule_get_num_strings(key, i, 0, r->tokens, r->len, grammar, l_str);
    if ((s + v) >= at) {
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
  for (;sum_rule + (small_s*rem) < at; small_s++);
  small_s --;
  assert(sum_rule + (small_s*rem) < at);
  return small_s;
}

void rule_get_num_strings_at(int key, int rule, int pos, int* tokens, int len, Grammar* grammar, int l_str, max_count_t at) {
  if (!len) return; // empty rule
  if (len == 1) {
    max_count_t v = key_get_num_strings(tokens[0], grammar, l_str); // if not tail
    if (v >= at) {
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
    if ((sum_rule + sr) >= at) {
      // We need to split here correctly. Which is the largest s_ so that
      // sum_rule + (s_ * rem) < at and sum_rule + ((s_+1)*rem) >=at ?
      // This would be the head the remainder would be the tail.
      max_count_t smallest_s_ = find_largest_s(sum_rule, s_, rem, at);
      key_get_num_strings_at(token, grammar, l_str_x, smallest_s_);

      max_count_t r = at - (sum_rule + smallest_s_*rem);

      // Now this remainder should go into rule at.
      rule_get_num_strings_at(key, rule, pos+1, tail, len-1, grammar, l_str - l_str_x, r);
      // We don't need to continue.
      return;
    }
    sum_rule += sr;
  }
  return;
}


void test_count_rules_at() {
#include "defs.h"
  int l_str = 1;
  max_count_t count = key_get_num_strings(0, &g, l_str);
  printf("%lu\n", count);
  for (int i = 1; i < 100; i++) {
    key_get_num_strings_at(0, &g, l_str, i); // this is l_str long
    printf("\n");
  }
}




/*
E1 = {
 '<start>': [['<E>']],
 '<E>': [['<F>', '*', '<E>'],
         ['<F>', '/', '<E>'],
         ['<F>']],
 '<F>': [['<T>', '+', '<F>'],
         ['<T>', '-', '<F>'],
         ['<T>']],
 '<T>': [['(', '<E>', ')'],
         ['<D>']],
 '<D>': [['0'], ['1']]
}

# Next, we implement `rule_get_num_strings()` which counts the number of strings
# of given size `l_str` that can be generated by a rule (an expansion of a nonterminal).
# Here, we treat each rule as a head followed by a tail. The token is the first
# symbol in the rule. The idea is that, the total number of strings that can be
# generated from a rule is the multiplication of the number of strings that can
# be generated from the head by the total strings that can be generated by the
# tail.
# 
# The complication is that we want to generate a specific size string. So, we
# split that size (`l_str`) between the head and tail and count strings
# generated by each possible split.


# ### Generation of strings
# 
# Let us next implement a way to generate all strings of a given
# size. Here, in `get_strings_of_length_in_definition()`, we pass in the key,
# the grammar and the length of the string expected.
# 
# For generating a string from a key, we first check if it is a `terminal`
# symbol. If it is, then there is only one choice. That symbol is the string.
# The constraint is that the length of the string should be as given by `l_str`.
# if not, then we find all the expansion rules of the corresponding definition
# and generate strings from each expansion of the given size `l_str`; the
# concatenation of which is the required string list.

def get_strings_of_length_in_definition(key, grammar, l_str):
    if not is_nonterminal(key):
        if l_str == len(key):
            return [key]
        else:
            return []
    # number strings in definition = sum of number of strings in rules
    rules = grammar[key]
    s = []
    for rule in rules:
        s_ = get_strings_of_length_in_rule(rule, grammar, l_str)
        s.extend(s_)
    return s
*/



void get_strings_of_length_in_definition_at_pos(int key, Grammar *grammar, int l_str, int pos) {
  return;
}

/*
# Next, we come to the rule implementation given by `get_strings_of_length_in_rule()`
# Here, we treat each rule as a head followed by a tail. The token is the first
# symbol in the rule. The idea is that, the strings that are generated from this
# rule will have one of the strings generated from the token followed by one of
# the strings generated from the rest of the rule. This also provides us with the
# base case. If the rule is empty, we are done.
# if it is not the base case, then we first extract the strings from the token
# head, then extract the strings from the tail, and concatenate them pairwise.
# 
# The complication here is the number of characters expected in the string. We
# can divide the number of characters --- `l_str` between the head and the tail.
# That is, if the string from head takes up `x` characters, then we can only
# have `l_str - x` characters in the tail. To handle this, we produce a loop
# with all possible splits between head and tail. Of course not all possible
# splits may be satisfiable. Whenever we detect an impossible split --- by
# noticing that `s_` is empty, we skip the loop.

def get_strings_of_length_in_rule(rule, grammar, l_str):
    if not rule: return []

    token, *tail = rule
    if not tail:
        return get_strings_of_length_in_definition(token, grammar, l_str)

    sum_rule = []
    for l_str_x in range(1,l_str+1): # inclusive
        s_ = get_strings_of_length_in_definition(token, grammar, l_str_x)
        if not s_: continue

        rem = get_strings_of_length_in_rule(tail, grammar, l_str - l_str_x)
        for s1 in s_:
            for sr in rem:
                sum_rule.append(s1 + sr)
    return sum_rule

# The problem with these implementations is that it is horribly naive. Each call
# recomputes the whole set of strings or the count again and again. However,
# many nonterminals are reused again and again, which means that we should be
# sharing the results. Let us see how we can memoize the results of these calls.
# 
# ## A Memoized Implementation.
# 
# ### Counting the number of strings
#
# We first define a data structure to keep the key nodes. Such nodes help us to
# identify the corresponding rules of the given key that generates strings of
# `l_str` size.

class KeyNode:
    def __init__(self, token, l_str, count, rules):
        self.token = token
        self.l_str = l_str
        self.count = count
        self.rules = rules

    def __str__(self):
        return "key: %s <%d> count:%d" % (repr(self.token), self.l_str, self.count)

    def __repr__(self):
        return "key: %s <%d> count:%d" % (repr(self.token), self.l_str, self.count)

# We also define a data structure to keep the rule nodes. Such rules contain
# both the head `token` as well as the `tail`, the `l_str` as well as the count

class RuleNode:
    def __init__(self, key, tail, l_str, count):
        self.key = key
        self.tail = tail
        self.l_str = l_str
        self.count = count
        assert count

    def __str__(self):
        return "head: %s tail: (%s) <%d> count:%d" % (repr(self.key.token), repr(self.tail), self.l_str, self.count)

    def __repr__(self):
        return "head: %s tail: (%s) <%d> count:%d" % (repr(self.key.token), repr(self.tail), self.l_str, self.count)

# globals

rule_strs = { }

key_strs = { }

EmptyKey = KeyNode(token=None, l_str=None, count=0, rules = None)

# ### Populating the linked data structure.
# 
# This follows the same skeleton as our previous functions. Firt the keys

def key_get_def(key, grammar, l_str):
    if (key, l_str) in key_strs: return key_strs[(key, l_str)]

    if key not in grammar:
        if l_str == len(key):
            key_strs[(key, l_str)] = KeyNode(token=key, l_str=l_str, count=1, rules = [])
            return key_strs[(key, l_str)]
        else:
            key_strs[(key, l_str)] = EmptyKey
            return key_strs[(key, l_str)]
    # number strings in definition = sum of number of strings in rules
    rules = grammar[key]
    s = []
    count = 0
    for rule in rules:
        s_s = rules_get_def(rule, grammar, l_str) # returns RuleNode (should it return array?)
        for s_ in s_s:
            assert s_.count
            count += s_.count
            s.append(s_)
    key_strs[(key, l_str)] = KeyNode(token=key, l_str=l_str, count=count, rules = s)
    return key_strs[(key, l_str)]

# Now the rules.

def rules_get_def(rule_, grammar, l_str):
    rule = tuple(rule_)
    if not rule: return []
    if (rule, l_str) in rule_strs: return rule_strs[(rule, l_str)]

    token, *tail = rule
    if not tail:
        s_ = key_get_def(token, grammar, l_str)
        if not s_.count: return []
        return [RuleNode(key=s_, tail=[], l_str=l_str, count=s_.count)]

    sum_rule = []
    count = 0
    for l_str_x in range(1, l_str+1):
        s_ = key_get_def(token, grammar, l_str_x)
        if not s_.count: continue

        rem = rules_get_def(tail, grammar, l_str - l_str_x)
        count_ = 0
        for r in rem:
            count_ += s_.count * r.count

        if count_:
            count += count_
            rn = RuleNode(key=s_, tail=rem, l_str=l_str_x, count=count_)
            sum_rule.append(rn)
    rule_strs[(rule, l_str)] = sum_rule
    return rule_strs[(rule, l_str)]

# We can of course extract the same things from this data structure
# 
# ### Count

def key_get_count(key_node):
    if not key_node.rules: return 1
    slen = 0
    for rule in key_node.rules:
        s = rule_get_count(rule)
        slen += s
    return slen

def rule_get_count(rule_node):
    slen = 0
    s_k = key_get_count(rule_node.key)
    for rule in rule_node.tail:
        s_t = rule_get_count(rule)
        slen = s_k * s_t
    if not rule_node.tail:
        slen += s_k
    return slen

# ### Strings

def key_extract_strings(key_node):
    # key node has a set of rules
    if not key_node.rules: return [key_node.token]
    strings = []
    for rule in key_node.rules:
        s = rule_extract_strings(rule)
        if s:
            strings.extend(s)
    return strings

def rule_extract_strings(rule_node):
    strings = []
    s_k = key_extract_strings(rule_node.key)
    for rule in rule_node.tail:
        s_t = rule_extract_strings(rule)
        for s1 in s_k:
            for s2 in s_t:
                strings.append(s1 + s2)
    if not rule_node.tail:
        strings.extend(s_k)
    return strings

# ### Random Access
# 
# But more usefully, we can now use it to randomly access any particular string

def key_get_string_at(key_node, at):
    assert at < key_node.count
    if not key_node.rules: return key_node.token
    at_ = 0
    for rule in key_node.rules:
        if at < (at_ + rule.count):
            return rule_get_string_at(rule, at - at_)
        else:
            at_ += rule.count
    return None

def rule_get_string_at(rule_node, at):
    assert at < rule_node.count
    if not rule_node.tail:
        s_k = key_get_string_at(rule_node.key, at)
        return s_k

    len_s_k = rule_node.key.count
    at_ = 0
    for rule in rule_node.tail:
        for i in range(len_s_k):
            if at < (at_ + rule.count):
                s_k = key_get_string_at(rule_node.key, i)
                return s_k + rule_get_string_at(rule, at - at_)
            else:
                at_ += rule.count
    return None


# ### Random Sampling
# 
# Once we have random access of a given string, we can turn it to random
# sampling. 

import random
# 

if __name__ == '__main__':
    key_node_g = key_get_def('<start>', E1, 5)
    print(key_node_g.count)
    at = random.randint(0,key_node_g.count)
    print('at:', at)
    strings = key_extract_strings(key_node_g)
    print("strting[%d]" % at, repr(strings[at]))
    string = key_get_string_at(key_node_g, at)
    print(repr(string))

# This is random sampling from restricted set --- the set of derivation strings 
# of a given length. How do we extend this to lengths up to a given length?
# The idea is that for generating strings of length up to `n`, we produce and
# use nonterminals that generate strings of length up to `n-x` where `x` is the
# length of first terminals in expansions. This means that we can build the
# `key_node` data structures recursively from 1 to `n`, and most of the parts
# will be shared between the `key_node` data structures of different lengths.

# Another issue this algorithm has is that it fails when there is left
# recursion. However, it is fairly easy to solve as I showed in a previous
# [post](/post/2020/03/17/recursive-descent-contextfree-parsing-with-left-recursion/).
# The idea is that there is a maximum limit to the number of useful recursions.
# Frost et. al.[^frost2007modular] suggests a limit of `m * (1 + |s|)` where `m`
# is the number of nonterminals in the grammar and `|s|` is the length of input.
# So, we use that here for limiting the recursion.

import bisect

class RandomSampleCFG:
    def __init__(self, grammar):
        self.grammar = grammar
        self.rule_strs = { }
        self.key_strs = { }
        self.EmptyKey = KeyNode(token=None, l_str=None, count=0, rules = None)
        self.ds = {}
        self.recursion_ctr = {}
        self.count_nonterminals = len(grammar.keys())

    def key_get_def(self, key, l_str):
        if (key, l_str) in self.key_strs: return self.key_strs[(key, l_str)]

        if key not in self.grammar:
            if l_str == len(key):
                self.key_strs[(key, l_str)] = KeyNode(token=key, l_str=l_str, count=1, rules = [])
                return self.key_strs[(key, l_str)]
            else:
                self.key_strs[(key, l_str)] = EmptyKey
                return self.key_strs[(key, l_str)]
        # number strings in definition = sum of number of strings in rules
        if key not in self.recursion_ctr: self.recursion_ctr[key] = 0

        self.recursion_ctr[key] += 1

        limit = self.count_nonterminals * (1 + l_str) # m * (1 + |s|)
        # remove left-recursive rules -- assumes no epsilon
        if self.recursion_ctr[key] > limit:
            rules = [r for r in self.grammar[key] if r[0] != key]
        else:
            rules = self.grammar[key] # can contain left recursion


        s = []
        count = 0
        for rule in rules:
            s_s = self.rules_get_def(rule, l_str) # returns RuleNode (should it return array?)
            for s_ in s_s:
                assert s_.count
                count += s_.count
                s.append(s_)
        self.key_strs[(key, l_str)] = KeyNode(token=key, l_str=l_str, count=count, rules = s)
        return self.key_strs[(key, l_str)]

    # Now the rules.

    def rules_get_def(self, rule_, l_str):
        rule = tuple(rule_)
        if not rule: return []
        if (rule, l_str) in self.rule_strs: return self.rule_strs[(rule, l_str)]

        token, *tail = rule
        if not tail:
            s_ = self.key_get_def(token, l_str)
            if not s_.count: return []
            return [RuleNode(key=s_, tail=[], l_str=l_str, count=s_.count)]

        sum_rule = []
        count = 0
        for l_str_x in range(1, l_str+1):
            s_ = self.key_get_def(token, l_str_x)
            if not s_.count: continue

            rem = self.rules_get_def(tail, l_str - l_str_x)
            count_ = 0
            for r in rem:
                count_ += s_.count * r.count

            if count_:
                count += count_
                rn = RuleNode(key=s_, tail=rem, l_str=l_str_x, count=count_)
                sum_rule.append(rn)
        self.rule_strs[(rule, l_str)] = sum_rule
        return self.rule_strs[(rule, l_str)]

    def key_get_string_at(self, key_node, at):
        assert at < key_node.count
        if not key_node.rules: return (key_node.token, [])
        at_ = 0
        for rule in key_node.rules:
            if at < (at_ + rule.count):
                return (key_node.token, self.rule_get_string_at(rule, at - at_))
            else:
                at_ += rule.count
        assert False

    def rule_get_string_at(self, rule_node, at):
        assert at < rule_node.count
        if not rule_node.tail:
            s_k = self.key_get_string_at(rule_node.key, at)
            return [s_k]

        len_s_k = rule_node.key.count
        at_ = 0
        for rule in rule_node.tail:
            for i in range(len_s_k):
                if at < (at_ + rule.count):
                    s_k = self.key_get_string_at(rule_node.key, i)
                    return [s_k] + self.rule_get_string_at(rule, at - at_)
                else:
                    at_ += rule.count
        assert False

    # produce a shared key forest.
    def produce_shared_forest(self, start, upto):
        for length in range(1, upto+1):
            if length in self.ds: continue
            key_node_g = self.key_get_def(start, length)
            count = key_node_g.count
            self.ds[length] = key_node_g
        return self.ds

    def compute_cached_index(self, n, cache):
        cache.clear()
        index = 0
        for i in range(1, n+1):
            c = self.ds[i].count
            if c:
                cache[index] = self.ds[i]
                index += c
        total_count = sum([self.ds[l].count for l in self.ds if l <= n])
        assert index == total_count
        return cache

    def get_total_count(self, cache):
        last = list(cache.keys())[-1]
        return cache[last].count + last


    # randomly sample from 1 up to `l` length.
    def random_sample(self, start, l, cache=None):
        assert l > 0
        if l not in self.ds:
            self.produce_shared_forest(start, l)
        if cache is None:
            cache = self.compute_cached_index(l, {})
        total_count = self.get_total_count(cache)
        choice = random.randint(0, total_count-1)
        my_choice = choice
        # get the cache index that is closest.
        index = bisect.bisect_right(list(cache.keys()), choice)
        cindex = list(cache.keys())[index-1]
        my_choice = choice - cindex # -1
        return choice, self.key_get_string_at(cache[cindex], my_choice)

    # randomly sample n items from 1 up to `l` length.
    def random_samples(self, start, l, n):
        cache = {}
        lst = []
        for i in range(n):
            lst.append(self.random_sample(start, l, cache))
        return lst

# Using it.

if __name__ == '__main__':
    rscfg = RandomSampleCFG(E1)
    max_len = 10
    rscfg.produce_shared_forest('<start>', max_len)
    for i in range(10):
        at = random.randint(1, max_len) # at least 1 length
        v, tree = rscfg.random_sample('<start>', at)
        string = tree_to_string(tree)
        print("mystring:", repr(string), "at:", v, "upto:", at)
*/

int
main(int argc, char* argv[]) {
  // test_tree_to_string();
  test_count_rules_at();
  return 0;
}
