#!/usr/bin/env python
"""
wnyc_sensors
======

Command line tool to dump data to sentry.  

"""

from setuptools import setup


setup(
    name='wnyc_sensors',
    version='0.0.0',
    author='Adam DePrince',
    author_email='adeprince@nypublicradio.org',
    description='Simple command line driven logging for sentry.',
    long_description=__doc__,
    py_modules = [
        "wnyc_sensors/server/packet_parser",
        ],
    packages = ["wnyc_sensors"],
    zip_safe=True,
    license='GPL',
    include_package_data=True,
    classifiers=[
        ],
    scripts = [],
    url = "https://github.com/adamdeprince/wnyc_sensors",
    install_requires = [
        ]
)

