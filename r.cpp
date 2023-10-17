#include <string.h>
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

char* tree_to_string(TreeNode* tree) {
    TreeNode* to_expand[MAX_TREE_NODES_IN_STACK]; // Array to store nodes to expand
    int to_expand_size = 1;
    to_expand[0] = tree;
    to_expand[1] = 0; // remove

    char *expanded = new char[MAX_STRING_SIZE]; // Array to store expanded characters
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
  char* v = tree_to_string(&root);
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

int rule_get_num_strings(Rule* rule, Grammar* grammar, int l_str);

int key_get_num_strings(int key, Grammar* grammar, int l_str) {
  if (!is_nonterminal(key))
    return l_str == 1 ? 1 : 0; // we assume every terminal is size 1
  int s = 0;
  Def* def = &grammar->defs[-key]; // indexed negative
  for (int i = 0; i < def->len; i++) {
    s += rule_get_num_strings(&def->rules[i], grammar, l_str);
  }
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

int rule_get_num_strings(Rule* rule, Grammar* grammar, int l_str);

int rule_get_num_strings_(int* tokens, int len, Grammar* grammar, int l_str) {
  if (!len) return 0; // empty rule
  if (len == 1) return key_get_num_strings(tokens[0], grammar, l_str); // if not tail

  int sum_rule = 0;
  int token = tokens[0];
  int *tail = tokens+1;
  for(int l_str_x = 1; l_str_x < l_str+1; l_str_x++) {
    int s_ = key_get_num_strings(token, grammar, l_str_x);
    if (s_ == 0) continue;

    int rem = rule_get_num_strings_(tail, len-1, grammar, l_str - l_str_x );
    sum_rule += s_ * rem;
  }
  return sum_rule;

}

int rule_get_num_strings(Rule* rule, Grammar* grammar, int l_str) {
  return rule_get_num_strings_(rule->tokens, rule->len, grammar, l_str);
}

void test_count_rules() {
#include "defs.h"
  /*
  int start = 0;
  int digits = -1;
  int digit = -2;
  int start_rule1_tokens[] = {digits};

  Rule start_rule1 = {
    .len = 1,
    .tokens = start_rule1_tokens
  };
  Rule start_rules[] = {start_rule1};

  Def def_start = {.name = "start",
    .len=1,
    .rules = start_rules
  };

  int digits_rule1_tokens[] = {digit, digits};
  int digits_rule2_tokens[] = {digit};
  Rule digits_rule1 = {
    .len = 2,
    .tokens = digits_rule1_tokens
  };
  Rule digits_rule2 = {
    .len = 1,
    .tokens = digits_rule2_tokens
  };

  Rule digits_rules[] = {digits_rule1, digits_rule2};

  Def def_digits = {.name="digits",
    .len=2,
    .rules = digits_rules
  };


  int digit_rule1_tokens[] = {'1'};
  Rule digit_rule1 = { .len = 1, .tokens = digit_rule1_tokens };

  int digit_rule2_tokens[] = {'2'};
  Rule digit_rule2 = { .len = 1, .tokens = digit_rule2_tokens };

  int digit_rule3_tokens[] = {'3'};
  Rule digit_rule3 = { .len = 1, .tokens = digit_rule3_tokens };

  int digit_rule4_tokens[] = {'4'};
  Rule digit_rule4 = { .len = 1, .tokens = digit_rule4_tokens };
 
  int digit_rule5_tokens[] = {'5'};
  Rule digit_rule5 = { .len = 1, .tokens = digit_rule5_tokens };


  int digit_rule6_tokens[] = {'6'};
  Rule digit_rule6 = { .len = 1, .tokens = digit_rule6_tokens };

  int digit_rule7_tokens[] = {'7'};
  Rule digit_rule7 = { .len = 1, .tokens = digit_rule7_tokens };

  int digit_rule8_tokens[] = {'8'};
  Rule digit_rule8 = { .len = 1, .tokens = digit_rule8_tokens };

  int digit_rule9_tokens[] = {'9'};
  Rule digit_rule9 = { .len = 1, .tokens = digit_rule9_tokens };
 
  int digit_rule0_tokens[] = {'0'};
  Rule digit_rule0 = { .len = 1, .tokens = digit_rule0_tokens };

 
  Rule digit_rules[] = {
    digit_rule0,
    digit_rule1,
    digit_rule2,
    digit_rule3,
    digit_rule4,
    digit_rule5,
    digit_rule6,
    digit_rule7,
    digit_rule8,
    digit_rule9
  };
  Def def_digit = {.name="digit",
    .len=10,
    .rules = digit_rules
  };


  Def my_defs[3] = { def_start, def_digits, def_digit };

  Grammar g = {
    .len = 3,
    .defs = my_defs
  };
  */

  int count = key_get_num_strings(0, &g, 2);
  printf("%d\n", count);

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
  test_count_rules();
  return 0;
}
