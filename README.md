python-ctokenizer
=================

A c-extension module for python that tokenizes unicode strings

Notes
=====

This is a fairly special-purpose tokenizer written in c. Requires input to be unicode. All characters are defined as alpha, numeric, or punctuation. Consecutive characters of the same type are kept together as a single token. There are several "special" token sequences, that if found, will be kept together (e.g <numeric> '.' <numeric>, to handle values like '23.4'; and <alpha> "'" <alpha>, keeping "don't" together as one word).

Future
======
Allow special sequences to be passed in
Support regular strings as well as unicode

