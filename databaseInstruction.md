CREATE DATABASE biblioteca;

CREATE TABLE usuarios(
    id SERIAL PRIMARY KEY,
    nome VARCHAR(100),
    uid VARCHAR(50) UNIQUE
);

INSERT INTO usuarios(nome, uid)
VALUES ('Augusto', 'A35F218C');

ALTER USER postgres
PASSWORD 'senha123';
