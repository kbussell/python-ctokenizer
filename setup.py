#!/usr/bin/env python

from distutils.core import setup, Extension

module1 = Extension('Tokenizer', sources = ['tokenizer.c'])

setup (name = 'Tokenizer',
        version = '1.0',
        description = 'special purpose line tokenizer',
        ext_modules = [module1],
        author = 'Keith Bussell')
