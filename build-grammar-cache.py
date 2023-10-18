def is_nonterminal(k): return (k[0], k[-1]) == ('<', '>')

# Data structures:
"""
#include <limits.h>
const int MAX_L_STR = %(max_l)d;
typedef unsigned long max_count_t;
typedef int max_len_t;
"""

# A huge array of grammar size * length of strings for storing rules.
# Each rule tail, that is token gets n entries for string length n.
"""
const int RULE_CACHE_SIZE = MAX_L_STR * %(grammar_size)d;
int get_rule_count_at_length(int key, int rule, int loc, max_len_t l_str) {
    // The location of the token
    int loc = 
}
int set_rule_count_at_length(int key, int rule, int loc, max_len_t l_str, max_count_t val) {
}

"""

# A smallish array of #keys * length of strings for storing keys
"""
const int KEY_CACHE_SIZE = MAX_L_STR * %(num_keys)d;
max_count_tkey_len_cache[KEY_CACHE_SIZE];

max_count_t UNINITIALIZED = ULONG_MAX;


void init_key_len_cache() {
    for(int i = 0; i < KEY_CACHE_SIZE; i++) key_len_cache[i] = UNINITIALIZED;
}

int get_key_count_at_length(int key, max_len_t l_str) {
    // start is 0. Each key has MAX_L_STR entries.
    int loc = (-key)*MAX_L_STR + l_str;
    max_count_t val = key_len_cache[loc];
    /*if (val != UNINITIALIZED) */
    return val;
}

void set_key_count_at_length(int key, max_len_t l_str, max_count_t val) {
    int loc = (-key)*MAX_L_STR + l_str;
    key_len_cache[loc] = val;
}
"""



def grammar_size(g):
    return sum(len(r) for k in g for r in g[k])

def build_grammar_cache(g, depth):

    """
int * grammar_cache[] = 
    """

#--------
def is_nonterminal(k):
    return (k[0], k[-1]) == ('<', '>')

def tokens(rule):
    return ', '.join(['UNINITIALIZED' for token in rule])


def print_def(k, i, rules):
    key = k[1:-1]
    key_dec = '%(key)s = -%(i)d' % {'key':key, 'i': i}

    rule_def = '// new rule'.join(['''
int Cache_%(key)s_rule%(rulei)d_tokens[] = {%(tokens)s};

Rule Cache_%(key)s_rule%(rulei)d = {
    .len = %(tokens_num)d,
    .tokens = Cache_%(key)s_rule%(rulei)d_tokens
};
''' % {'key':key, 'rulei':i, 'tokens_num': len(rule), 'tokens':tokens(rule)} for i,rule in enumerate(rules)])
    rules_arr = ', '.join([ 'Cache_%s_rule%d' % (key, i) for i,rule in enumerate(rules)])
    s = """
%(rule_def)s

Rule Cache_%(key)s_rules[] = {%(rules_arr)s};

Def Cache_def_%(key)s = {.name = "%(key)s",
    .len=%(num_rules)d,
    .rules = Cache_%(key)s_rules
};

    """ % {'key':key, 'num_rules': len(rules), 'rules_arr': rules_arr, 'rule_def': rule_def}
    print(s)

def convert_key(k, i, defs):
    print_def(k, i, defs)

def compile_grammar(g):
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
Def Cache_my_defs[] = {%(key_desc)s};
Grammar Cache_G = {
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
    g = {
"<start>" : [["<digits>"]],
"<digits>" : [["<digit>", "<digits>"],
        ["<digit>"]],
"<digit>" : [[str(i)] for i in range(10)]
}
    compile_grammar(g)
