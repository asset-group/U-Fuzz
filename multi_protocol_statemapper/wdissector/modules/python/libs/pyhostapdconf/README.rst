=============
pyhostapdconf
=============

This package contains a library for working with hostapd configuration file in
Python. It is intended for configuring simple access points and supports the
folowing configuration:

- Interface selection
- Driver selection (nl80211 and rtl871xdrv, as supported)
- SSID
- Country code
- Channel
- WPA configuration (passphrase, WPA mode)
- Maximum client limit
- Hidden SSID

Configuration class
===================

To load the configuration file, use the ``HostapdConf`` class::

    >>> from hostapdconf.parser import HostapdConf
    >>> conf = HostapdConf('example/hostapd_example.conf')

This class is a generic configuration parser/writer and doesn't know much about
how configuration options work. It behaves like a dictionary, and you can look
up keys using subscript notation. For example::


    >>> conf['interface']
    'wlan0'

If a key is not found, a ``KeyError`` is raised. ::

    >>> conf['driver']
    KeyError: '<HostapdConf "interface=wlan0...">' has no key 'driver'

You can set any key by assigning::

    >>> conf['driver'] = 'nl80211'

You can update multiple values by calling the ``update()`` method with a
dictionary as its argument, just like with Python dictionaries. Finally, if you
want to save the configuration file, you can use the ``write()`` method::

    >>> conf.write()

.. warning::
    Calling ``write()`` overwrites the file that was loaded and comments are
    **not** preserved.

If you wish to save the file to another location, simply pass the path to the
``write()`` method. To reload the configuration, you can use the ``reload()``
method.

``HostapdConf`` class will preserve the order of the configuration keys as much
as possible, but it does not offer methods for inserting configuration options
at random locations.

Helper functions
================

To help with common configuration tasks, there is a number of helper methods.

Helpers are found in the ``hosapdconf.helpers`` module. ::

    >>> from hostapdconf import helpers as ha

Each helper function takes the configuration object as first argument followed
by arguments specific to the setting. Here is a quick overview of the helper
methods::

    >>> ha.set_ssid(conf, 'Foobar')
    >>> ha.hide_ssid(conf)
    >>> ha.reveal_ssid(conf)
    >>> ha.set_iface(conf, 'wlan2')
    >>> ha.set_driver(conf, ha.REALTEK)  # or ha.STANDARD
    >>> ha.set_channel(conf, 2)
    # wpa_modes supported: ha.WPA1_ONLY, ha.WPA2_ONLY, ha.WPA_BOTH
    >>> ha.enable_wpa(conf, passphrase='SECRET', wpa_mode=ha.WPA2_ONLY)
    >>> ha.disable_wpa(conf)
    >>> ha.set_country(conf, 'de')

License
=======

pyhostapdconf is released under GPLv3. Please see the ``COPYING`` file in the
source tree.
