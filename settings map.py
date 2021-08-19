#!/usr/bin/env python3

import sys
import configparser

# usage "settings map.py" "settings ini" "settings header" "settings c file"

h = open(sys.argv[2], 'w')
c = open(sys.argv[3], 'w')

config = configparser.RawConfigParser()
config.optionxform = lambda option: option
config.read(sys.argv[1])
enums = []
sizes = []
addresses = []
address = 0
for key in config['fields sizes']:
    enums.append('es' + key)
    size = int(config['fields sizes'][key])
    sizes.append(size)
    addresses.append(address)
    address += size

h.write("""
#ifndef PROJECT_SETTINGS_H_
#define PROJECT_SETTINGS_H_

#include <stdint.h>

typedef enum {
    %s,
    esAmount
} eSetting;

#endif /* PROJECT_SETTINGS_H_ */
""" % ',\n    '.join(enums))
h.close()

c.write("""
#include "project_settings.h"
#include "KagamiCore/defines.h"
#include <stdint.h>

const static uint16_t settingsSize[esAmount] = {
    %s
};

const static uint16_t settingsAddress[esAmount] = {
    %s
};
""" % (
    ',\n    '.join(['[%s] = %d' % (enums[i], sizes[i]) for i in range(len(enums))]),
    ',\n    '.join(['[%s] = %d' % (enums[i], addresses[i]) for i in range(len(enums))])
))
c.close()
