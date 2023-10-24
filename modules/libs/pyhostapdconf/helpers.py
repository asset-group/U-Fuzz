"""
Configuration fragment generator functions

2016, Outernet Inc
Some rights reserved.

This software is free software licensed under the terms of GPLv3. See COPYING
file that comes with the source code, or http://www.gnu.org/licenses/gpl.txt.
"""


STANDARD = 'nl80211'
REALTEK = 'rtl871xdrv'
DRIVERS = (STANDARD, REALTEK)

WPA_NONE = 0
WPA1_ONLY = 1
WPA2_ONLY = 2
WPA_BOTH = 3
WPA_MODES = (WPA1_ONLY, WPA2_ONLY, WPA_BOTH)


class ConfigurationError(Exception):
    pass


def _safe_del(conf, key):
    """ Safely remove key and do not raise if key is missing """
    try:
        del conf[key]
    except KeyError:
        pass


def set_ssid(conf, ssid):
    """ Set SSID on the specified configuration object. """
    conf['ssid'] = ssid


def set_iface(conf, iface):
    """ Set interface on the specified configuration object. """
    conf['interface'] = iface


def set_country(conf, code):
    """ Set country code on the specified configuration object. """
    if not code:
        _safe_del(conf, 'country_code')
        return
    conf['country_code'] = code


def set_channel(conf, channel):
    """ Set channel on the specified configuration object. """
    if not channel:
        conf['channel'] = 6
    if conf.get('country_code', '').upper() in ['US', 'CA']:
        max_ch = 11
    else:
        max_ch = 13
    if channel < 1 or channel > max_ch:
        raise ConfigurationError('Channel {} is out of range'.format(channel))
    conf['channel'] = channel


def enable_wpa(conf, passphrase, wpa_mode=WPA_BOTH):
    """ Enable WPA-PSK on configuration object.

    More info on these options can be found here:

    http://www.ibm.com/developerworks/library/l-wifiencrypthostapd/index.html
    """
    conf['auth_algs'] = 1
    conf['wpa'] = wpa_mode
    conf['wpa_passphrase'] = passphrase
    conf['wpa_key_mgmt'] = 'WPA-PSK'
    conf['wpa_pairwise'] = 'CCMP TKIP'
    conf['rsn_pairwise'] = 'CCMP'
    conf['ieee8021x'] = 0


def disable_wpa(conf):
    for k in ['auth_algs', 'wpa', 'wpa_passphrase', 'wpa_key_mgmt',
              'wpa_pairwise', 'rsn_pairwise', 'ieee8021x']:
        _safe_del(conf, k)


def limit_clients(conf, num):
    """ Limit number of clients to specified number """
    if num < 1:
        raise ConfigurationError('Client limit too low')
    conf['max_num_sta'] = num
    conf['no_probe_resp_if_max_sta'] = 1


def hide_ssid(conf):
    """ Do not broadcast the SSID """
    conf['ignore_broadcast_ssid'] = 1


def reveal_ssid(conf):
    """ Unhide the SSID """
    _safe_del(conf, 'ignore_broadcast_ssid')


def set_driver(conf, driver):
    if driver not in DRIVERS:
        raise ConfigurationError('Invalid driver selection')
    conf['driver'] = driver
