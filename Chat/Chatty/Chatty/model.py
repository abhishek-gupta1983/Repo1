# -*- coding: utf-8 -*-
"""This module contains the data model of the application."""


import pkg_resources
pkg_resources.require('SQLAlchemy>=0.4.3')

from turbogears.database import get_engine, metadata, session
# import the standard SQLAlchemy mapper
from sqlalchemy.orm import mapper
# To use the session-aware mapper use this import instead
# from turbogears.database import session_mapper as mapper
# import some basic SQLAlchemy classes for declaring the data model
# (see http://www.sqlalchemy.org/docs/05/ormtutorial.html)
from sqlalchemy import Table, Column, ForeignKey
from sqlalchemy.orm import relation
# import some datatypes for table columns from SQLAlchemy
# (see http://www.sqlalchemy.org/docs/05/reference/sqlalchemy/types.html for more)
from sqlalchemy import String, Unicode, Integer, DateTime


# your data tables
# http://www.sqlalchemy.org/docs/05/metadata.html

# your_table = Table('yourtable', metadata,
#     Column('my_id', Integer, primary_key=True)
# )


# your model classes
# http://www.sqlalchemy.org/docs/05/ormtutorial.html#define-a-python-class-to-be-mapped

# class YourDataClass(object):
#     pass


# set up mappers between your data tables and classes
# http://www.sqlalchemy.org/docs/05/mappers.html

# mapper(YourDataClass, your_table)




# functions for populating the database

def bootstrap_model(clean=False):
    """Create all database tables and fill them with default data.

    This function is run by the 'bootstrap' function from the command module.
    By default it creates all database tables for your model.

    You can add more functions as you like to add more boostrap data to the
    database or enhance the function below.

    If 'clean' is True, all tables defined by your model will be dropped before
    creating them again.

    """
    create_tables(clean)

def create_tables(drop_all=False):
    """Create all tables defined in the model in the database.

    Optionally drop existing tables before creating them.

    """
    get_engine()
    if drop_all:
        print "Dropping all database tables defined in model."
        metadata.drop_all()
    metadata.create_all()

    print "All database tables defined in model created."

