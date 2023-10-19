GrammarE = {
        '<start>': [['<E>']],
        '<E>': [['<F>', '*', '<E>'],
                ['<F>', '/', '<E>'],
                ['<F>']],
        '<F>': [['<T>', '+', '<F>'],
                ['<T>', '-', '<F>'],
                ['<T>']],
        '<T>': [['(', '<E>', ')'],
                ['<Ds>']],
        '<Ds>':[['<D>', '<Ds>'], ['<D>']],
        '<D>': [[str(i)] for i in range(10)]
        }

Grammar = GrammarE

if __name__ == '__main__':
    import json
    print(json.dumps(Grammar, indent=2))
