"""
Save messages to the database.
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

# path to the database
DB_PATH = "/opt/suns_monitor/db/suns_monitor.db"

# create the database
Base = declarative_base()
class Data(Base):
    """
    Class to represent the data table.
    """
    # nome da tabela
    __tablename__ = 'data'
    # colunas
    # Identificador único de cada linha
    id = Column(Integer, primary_key=True)
    # Modelo SunSpec
    model =  Column(Integer)
    # Instância do modelo
    instance =  Column(Integer)
    # Ponto do modelo
    point =  Column(String)
    # Valor do ponto. 
    # Armazenado como JSON para aceitar diferentes tipos de dados
    # Manter o seguinte padrão: {"value": <valor>}
    value = Column(JSON)
    # Data e hora da leitura
    timestamp = Column(Integer)  

    def __repr__(self):
        return f'{{"model":{self.model}, "instance":{self.instance}, "point":{self.point}, "value":{self.value}, "timestamp":{datetime.fromtimestamp(self.timestamp)}}}'

def start_db_session():
    """
    Start a session with the database.
    """
    print(f"DB_PATH: {DB_PATH}")
    engine = create_engine(f'sqlite:///{DB_PATH}', echo=False)
    Session = sessionmaker(bind=engine)
    session = Session()
    Base.metadata.create_all(engine)
    return session

def save_data(model, instance, point, value, timestamp, session):
    """
    Save data to the database.
    """
    try:
        # create the data object
        data = Data(model = model,
            instance = instance,
            point = point,
            value = {"value": value},
            timestamp = timestamp)
        # save the data
        session.add(data)
        # commit the data
        session.commit()
        print(data)
    except Exception as e:
        print(f'save_data error: {e}')