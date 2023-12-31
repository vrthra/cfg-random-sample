def is_nonterminal(k): return (k[0], k[-1]) == ('<', '>')

# Data structures:
PRE="""
#include <limits.h>
const int MAX_L_STR = %(max_l)d;
typedef unsigned long max_count_t;
typedef int max_len_t;

struct Cache_Token {
    max_count_t items[MAX_L_STR];
};

struct Cache_Rule {
  const char* name;
  int len;
  Cache_Token* tokens;
};

struct Cache_Def {
  const char* name;
  int len;
  Cache_Rule* rules;
};

struct Cache_Grammar {
  int len;
  Cache_Def* defs;
};
"""

# A huge array of grammar size * length of strings for storing rules.
# Each rule tail, that is token gets n entries for string length n.
POST = """
max_count_t get_rule_count_at_length(int key, int rule, int loc, max_len_t l_str) {
    return Cache_G.defs[-key].rules[rule].tokens[loc].items[l_str];
}
void set_rule_count_at_length(int key, int rule, int loc, max_len_t l_str, max_count_t val) {
    Cache_G.defs[-key].rules[rule].tokens[loc].items[l_str] = val;
}

"""

# A smallish array of #keys * length of strings for storing keys
PRE += """
const max_count_t UNINITIALIZED = ULONG_MAX;
const int KEY_CACHE_SIZE = MAX_L_STR * %(num_keys)d;
max_count_t key_len_cache[KEY_CACHE_SIZE] = {%(key_len_cache_init)s};

"""
POST +="""
max_count_t get_key_count_at_length(int key, max_len_t l_str) {
    // start is 0. Each key has MAX_L_STR entries.
    int loc = (-key)*MAX_L_STR + l_str;
    max_count_t val = key_len_cache[loc];
    return val;
}

void set_key_count_at_length(int key, max_len_t l_str, max_count_t val) {
    int loc = (-key)*MAX_L_STR + l_str;
    key_len_cache[loc] = val;
}
"""

def tokens(key, i, rule):
    return ','.join(["token_%s_%d_%d" % (key, i, j) for j,token in enumerate(rule)])

def print_def(k_, k_i, rules):
    key = k_[1:-1]

    rule_def = '// new rule'.join(['''
Cache_Token Cache_%(key)s_rule%(rulei)d_tokens[] = {%(tokens)s};

Cache_Rule Cache_%(key)s_rule%(rulei)d = {
    .name = "%(key)s %(rulei)d",
    .len = %(tokens_num)d,
    .tokens = Cache_%(key)s_rule%(rulei)d_tokens
};
''' % {'key':key, 'rulei':i, 'tokens_num': len(rule), 'tokens':tokens(key, i, rule)} for i,rule in enumerate(rules)])
    rules_arr = ', '.join([ 'Cache_%s_rule%d' % (key, i) for i,rule in enumerate(rules)])
    s = """
%(rule_def)s

Cache_Rule Cache_%(key)s_rules[] = {%(rules_arr)s};

Cache_Def Cache_def_%(key)s = {
    .name = "%(key)s",
    .len=%(num_rules)d,
    .rules = Cache_%(key)s_rules
};

    """ % {'key':key, 'num_rules': len(rules), 'rules_arr': rules_arr, 'rule_def': rule_def}
    print(s)

def convert_key(k, i, defs):
    print_def(k, i, defs)

def compile_grammar(g, max_l_str):
    uninitialized_arr = '{%s}' % (','.join(['UNINITIALIZED' for i in range(max_l_str)]))

    s = ""
    for k_ in g:
        key = k_[1:-1]
        for j,rule in enumerate(g[k_]):
            for k,t in enumerate(rule):
                s += "Cache_Token token_%s_%d_%d = {.items=%s};\n" % (key, j, k, uninitialized_arr)
    print(s)

    pre = ''
    for i,k in enumerate(g):
        key = k[1:-1]
        key_desc = '%(key)s = -%(i)d' % {'key':key, 'i': i}
        pre += """
//const int %(key_desc)s;
    """ % {'key_desc': key_desc}
    print(pre)

    for i,k in enumerate(g):
        convert_key(k, i, g[k])

    key_desc = ', '.join(['Cache_def_%(key)s' % {'key':k[1:-1]} for k in g])

    post = """
Cache_Def Cache_my_defs[] = {%(key_desc)s};
Cache_Grammar Cache_G = {
    .len = %(n_defs)d,
    .defs = Cache_my_defs
};""" % {'key_desc': key_desc, 'n_defs': len(g.keys())}
    print(post)

if __name__ == '__main__':
    # IMPORTANT. We assume <start> is the first key, and hence gets 0 as its
    # index. Please check.
    import sys
    import json
    #with open(sys.argv[1]) as f:
    #    gs = f.read()
    #    g = json.loads(gs)
    #g = {
    #    "<start>" : [["<digits>"]],
    #    "<digits>" : [["<digit>", "<digits>"],
    #        ["<digit>"]],
    #        "<digit>" : [[str(i)] for i in range(10)]
    #}
    import grammar
    g = grammar.Grammar

    max_l_str = int(sys.argv[1])

    print(PRE % {'max_l' : max_l_str, 'num_keys': len(g.keys()), 'key_len_cache_init': ', '.join(['UNINITIALIZED' for i in range(max_l_str*len(g.keys()))]) })
    compile_grammar(g, max_l_str)
    print(POST % {'max_l' : max_l_str, 'num_keys': len(g.keys())})
