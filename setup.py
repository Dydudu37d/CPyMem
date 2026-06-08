from setuptools import setup, Extension

setup(
    name="cpymem",
    version="0.1",
    ext_modules=[Extension("cpymem", ["CPyMem.c"])]
)

