#!/usr/bin/env python
"""
wnyc_sensors
======

Command line tool to dump data to sentry.  

"""

from setuptools import setup


setup(
    name='wnyc-sensors',
    version='0.0.0',
    author='Adam DePrince',
    author_email='adeprince@nypublicradio.org',
    description='Simple command line driven logging for sentry.',
    long_description=__doc__,
    py_modules = [
        "wnyc_sensors/server/udp_handler",
        "wnyc_sensors/server/tcp_handler",
        "wnyc_sensors/server/udp_sender",
        "wnyc_sensors/server/packet_parser",
        "wnyc_sensors/server/__init__",
        "wnyc_sensors/__init__",
        ],
    packages = ["wnyc_sensors"],
    zip_safe=True,
    license='GPL',
    include_package_data=True,
    classifiers=[
        ],
    scripts = ['scripts/wnyc_sensor_server',
               'scripts/wnyc_fake_sensor',
               'scripts/wnyc_smsgateway_server',
               ],
    url = "https://github.com/adamdeprince/wnyc_sensors",
    install_requires = [
        ]
)

