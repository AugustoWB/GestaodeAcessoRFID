import serial
import psycopg2

arduino = serial.Serial(
    '/dev/ttyACM0',
    9600,
    timeout=1
)

con = psycopg2.connect(
    host="localhost",
    database="biblioteca",
    user="postgres",
    password="senha"
)

cur = con.cursor()

print("Servidor iniciado")

while True:

    linha = (
        arduino.readline()
        .decode()
        .strip()
    )

    if not linha:
        continue

    print("Recebido:", linha)

    if linha.startswith("UID:"):

        uid = linha.replace(
            "UID:",
            ""
        ).upper()

        cur.execute(
            """
            SELECT *
            FROM usuarios
            WHERE UPPER(uid)=%s
            """,
            (uid,)
        )

        usuario = cur.fetchone()

        if usuario:

            print("AUTORIZADO")

            arduino.write(
                b"OK\n"
            )

        else:

            print("NEGADO")

            arduino.write(
                b"NEGADO\n"
            )
