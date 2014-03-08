#!/usr/bin/env python
import logging, logging.handlers
import locale
import os

locale.setlocale(locale.LC_ALL, '')
logging.raiseExceptions = 0
log = logging.getLogger("")  #root logger

class logconf:
    def __init__(self,logFileName):
        logfile = "D:\\Temp\\"+logFileName # TODO: make me configurable.
        log.setLevel(logging.DEBUG)
        hdlr = logging.FileHandler(logfile, "a")            
        fmt = logging.Formatter("%(asctime)s [%(filename)s] - [%(lineno)4d] [%(levelname)-5s] %(message)s", "%x %X")
        hdlr.setFormatter(fmt)
        if not log.handlers:
            log.addHandler(hdlr)
        log.info("Chatty logging initialized")
