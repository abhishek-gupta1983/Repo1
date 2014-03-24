from sqlalchemy.ext.declarative import declarative_base

__author__ = 'Administrator'

__all__ = ['model_init', "TB1", "TB2"]
from sqlalchemy import Table, Column, ForeignKey
from sqlalchemy import String, Integer
from sqlalchemy.orm import mapper, clear_mappers, relation
class TB1(object):
    def __init__(self, c1):
        self.c1 = c1


class TB2(object):
    def __init__(self, tb1_id,  c1):
        self.c1 = c1
        self.tb1_id = tb1_id

class model_init(object):
    def __init__(self, *args, **kargs):
        print kargs
        if 'metadata' not in kargs.keys():
            print "Metadata cannot be none."
            return
        sa_mapper = mapper
        if 'sa_mapper' in kargs.keys():
            sa_mapper = kargs['sa_mapper']
        print kargs['metadata']
        metadata = kargs['metadata']
        print sa_mapper
        self.tb1 = Table('tb1', metadata,
                    Column('id', Integer, primary_key=True),
                    Column('c1', String(length=255))
        )

        self.tb2 = Table('tb2', metadata,
                    Column('id', Integer, primary_key=True),
                    Column('tb1_id', Integer, ForeignKey('tb1.id')),
                    Column('c1', String(length=255)),
                         extend_existing=True,
        )

        clear_mappers()

        sa_mapper(TB1, self.tb1, properties={'tb2': relation(TB2, backref="tb2")})
        sa_mapper(TB2, self.tb2)
