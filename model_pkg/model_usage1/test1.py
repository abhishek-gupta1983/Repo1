__author__ = 'Administrator'

import os, sys, inspect
currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
print parentdir
sys.path.insert(0, parentdir)
from model import plain_usage
from model import tg_usage
from model.schema_decl.schema_declarative import TB2
from model.schema_decl.schema_declarative import TB1
if __name__ == "__main__":
    pu = plain_usage.plain_init()
    tu = tg_usage.tg_init()
    tb11 = TB1(c1="Abhishek")
    pu.session.add(tb11)
    pu.session.flush()
    tb22 = TB2(tb1_id=tb11.id, c1="gupta")
    pu.session.add(tb22)
    pu.session.flush()

    data = pu.session.query(TB1).all()
    print data[0].tb2[0].c1
