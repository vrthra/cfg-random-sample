
"""
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
"""

def is_nonterminal(k):
    return (k[0], k[-1]) == ('<', '>')

def tokens(rule):
    return ', '.join([token[1:-1] if is_nonterminal(token) else "'%s'" % token for token in rule])


def print_def(k, i, rules):
    key = k[1:-1]
    key_dec = '%(key)s = -%(i)d' % {'key':key, 'i': i}

    rule_def = '// new rule'.join(['''
int %(key)s_rule%(rulei)d_tokens[] = {%(tokens)s};

Rule %(key)s_rule%(rulei)d = {
    .len = %(tokens_num)d,
    .tokens = %(key)s_rule%(rulei)d_tokens
};
''' % {'key':key, 'rulei':i, 'tokens_num': len(rule), 'tokens':tokens(rule)} for i,rule in enumerate(rules)])
    rules_arr = ', '.join([ '%s_rule%d' % (key, i) for i,rule in enumerate(rules)])
    s = """
%(rule_def)s

Rule %(key)s_rules[] = {%(rules_arr)s};

Def def_%(key)s = {.name = "%(key)s",
    .len=%(num_rules)d,
    .rules = %(key)s_rules
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
const int %(key_desc)s;
    """ % {'key_desc': key_desc}
    print(pre)

    for i,k in enumerate(g):
        convert_key(k, i, g[k])

    key_desc = ', '.join(['def_%(key)s' % {'key':k[1:-1]} for k in g])

    post = """
Def my_defs[] = {%(key_desc)s};
Grammar g = {
    .len = %(n_defs)d,
    .defs = my_defs
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
