__author__ = 'Administrator'

from sqlalchemy import MetaData, create_engine, orm
from sqlalchemy.pool import QueuePool
from schema_decl.schema_declarative import model_init

class plain_init(model_init):
    def __init__(self, *args, **kargs):
        self.engine = create_engine('mysql://web_backend:web_backend@localhost:3306/test_model', echo=False,
                                    pool_size=5, max_overflow=5, poolclass=QueuePool)
        self.metadata = MetaData(bind=self.engine)
        # Set up the session
        self.session_manager = orm.sessionmaker(bind=self.engine, autoflush=True, autocommit=True,
                                                expire_on_commit=True)
        self.session = self.session = orm.scoped_session(self.session_manager)

        super(plain_init, self).__init__(metadata=self.metadata,)
        print self.tb1

if __name__ == "__main__":
    plain = plain_init()