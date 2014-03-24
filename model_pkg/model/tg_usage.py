__author__ = 'Administrator'

from turbogears.database import metadata
from schema_decl.schema_declarative import model_init

class tg_init(model_init):
    def __init__(self, *args, **kargs):
        super(tg_init, self).__init__(*args, **kargs)

if __name__ == "__main__":
    tg_usage = tg_init()