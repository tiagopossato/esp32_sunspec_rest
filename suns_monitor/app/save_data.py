"""
S save messages to the database.
"""
import os
import sys
import json
from datetime import datetime
from time import sleep
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, Integer, JSON, DateTime, String

DB_PATH = "/opt/suns_monitor/db/suns_monitor.db"

Base = declarative_base()
class Data(Base):
    __tablename__ = 'data'
    id = Column(Integer, primary_key=True)
    model =  Column(Integer)
    instance =  Column(Integer)
    point =  Column(String)
    value = Column(JSON)
    timestamp = Column(Integer)  

    def __repr__(self):
        return f'{{"model":{self.model}, "instance":{self.instance}, "point":{self.point}, "value":{self.value}, "timestamp":{datetime.fromtimestamp(self.timestamp)}}}'

def start_db_session():
    print(f"DB_PATH: {DB_PATH}")
    engine = create_engine(f'sqlite:///{DB_PATH}', echo=False)
    Session = sessionmaker(bind=engine)
    session = Session()
    Base.metadata.create_all(engine)
    return session

def save_data(model, instance, point, value, timestamp, session):
    try:
        data = Data(model = model,
            instance = instance,
            point = point,
            value = {"value": value},
            timestamp = timestamp)
        session.add(data)
        session.commit()
        print(data)
    except Exception as e:
        print(f'save_data error: {e}')