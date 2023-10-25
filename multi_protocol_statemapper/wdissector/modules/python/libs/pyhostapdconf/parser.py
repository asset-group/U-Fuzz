"""
Configuration parser functions

2016, Outernet Inc
Some rights reserved.

This software is free software licensed under the terms of GPLv3. See COPYING
file that comes with the source code, or http://www.gnu.org/licenses/gpl.txt.
"""

import re
from collections import OrderedDict


NEWLINE_RE = re.compile(r'(\n\r|\r|\n)+')


class CommentOrBlankError(Exception):
    """ Parsed line is a comment or a blank line """
    pass


class HostapdConf(object):
    """
    This class provides basic methods for working with hostapd configuration
    files. Functionality offered by this class includes parsing, serializing,
    and persisting configuration.

    This class provides a subset of ``dict`` methods that allows subscript
    access to configuration keys, updating multiple keys, and so on.
    """

    def __init__(self, path=None, defaults={}):
        self.path = path
        self._data = OrderedDict()
        self.reload()

    def reload(self):
        """ Load the configuration from a given path """
        if not self.path:
            return
        with open(self.path, 'r') as f:
            for l in f:
                try:
                    key, val = self.parse_line(l)
                except CommentOrBlankError:
                    continue
                self._data[key] = val

    def serialize(self):
        """
        Serialize the data into "key=value" format.
        """
        s = []
        for key in self._data:
            val = self.clean_value(self._data.get(key, ''))
            s.append('{}={}'.format(key, val))
        return '\n'.join(s)

    def write(self, path=None, header=None):
        """"
        Write the configuration to a file.
        """
        if not self.path and not path:
            raise RuntimeError(
                "No path specified, and none bound to '{}'".format(self))
        with open(path or self.path, 'w') as f:
            if header:
                f.write(header)
            f.write(self.serialize())

    def get(self, key, default=None):
        """
        Get the value of a specified key or default value if key is missing.
        Works the same way as ``dict.get()``.
        """
        return self._data.get(key, default)

    def update(self, other):
        """
        Update multiple keys. Works the same way as ``dict.update()``.
        """
        self._data.update(other)

    def __getitem__(self, key):
        try:
            return self._data[key]
        except KeyError:
            raise KeyError("'{}' has no key '{}'".format(self, key))

    def __setitem__(self, key, val):
        self._data[key] = str(val)

    def __delitem__(self, key):
        del self._data[key]

    def __contains__(self, key):
        return key in self._data

    def __str__(self):
        return '<HostapdConf "{}">'.format(self.serialize())

    @staticmethod
    def clean_value(value):
        """ Strip newline characters, and trailing and leading whitespace. """
        return NEWLINE_RE.sub(' ', str(value)).strip()

    @staticmethod
    def parse_line(line):
        """ Parse a single line. """
        line = line.strip()
        if '=' not in line or line.startswith('#'):
            raise CommentOrBlankError()
        key, val = line.split('=', 1)
        key = key.strip()
        if not key:
            raise CommentOrBlankError()
        val = val.strip()
        return key, val
