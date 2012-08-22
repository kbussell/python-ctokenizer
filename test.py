#!/usr/bin/env python

__test__ = {'tokenizer_tests': """
>>> from Tokenizer import tokenize
>>> tokenize(u"This is a test")
(u'This', u'is', u'a', u'test')

>>> tokenize(u"can't touch this")
(u"can't", u'touch', u'this')

>>> tokenize(u'a2a')
(u'a', u'2', u'a')

>>> tokenize(u'2.2')
(u'2.2',)

>>> tokenize(u'a.a')
(u'a', u'.', u'a')

>>> tokenize(u'a2.2a')
(u'a', u'2.2', u'a')
"""}

def _test():
    import doctest
    doctest.testmod()

if __name__ == "__main__":
    _test()