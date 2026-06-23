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
    password="senha123"
)

cur = con.cursor()

print("=================================")
print("Servidor Biblioteca Iniciado")
print("=================================")

while True:

    try:

        linha = (
            arduino.readline()
            .decode()
            .strip()
        )

        if not linha:
            continue

        print("[SERIAL]", linha)

        # =====================
        # SENHA ERRADA
        # =====================

        if linha == "SENHA_ERRADA":

            print("[NEGADO] Senha incorreta")

            cur.execute(
                """
                INSERT INTO acessos
                (nome, uid, resultado)
                VALUES (%s,%s,%s)
                """,
                (
                    "DESCONHECIDO",
                    "SEM_RFID",
                    "NAO_AUTORIZADO"
                )
            )

            con.commit()

            continue

        # =====================
        # VISITANTE
        # =====================

        if linha == "VISITANTE":

            print("[VISITANTE] Entrada liberada")

            arduino.write(b"OK\n")

            cur.execute(
                """
                INSERT INTO acessos
                (nome, uid, resultado)
                VALUES (%s,%s,%s)
                """,
                (
                    "VISITANTE",
                    "VISITANTE",
                    "AUTORIZADO"
                )
            )

            con.commit()

            continue

        # =====================
        # RFID
        # =====================

        if linha.startswith("UID:"):

            uid = (
                linha
                .replace("UID:", "")
                .upper()
            )

            cur.execute(
                """
                SELECT nome, uid
                FROM usuarios
                WHERE UPPER(uid)=%s
                """,
                (uid,)
            )

            usuario = cur.fetchone()

            if usuario:

                nome = usuario[0]

                print(
                    f"[OK] Usuario: {nome}"
                )

                arduino.write(
                    b"OK\n"
                )

                cur.execute(
                    """
                    INSERT INTO acessos
                    (nome, uid, resultado)
                    VALUES (%s,%s,%s)
                    """,
                    (
                        nome,
                        uid,
                        "AUTORIZADO"
                    )
                )

                con.commit()

            else:

                print(
                    "[NEGADO] RFID nao cadastrado"
                )

                arduino.write(
                    b"NEGADO\n"
                )

                cur.execute(
                    """
                    INSERT INTO acessos
                    (nome, uid, resultado)
                    VALUES (%s,%s,%s)
                    """,
                    (
                        "DESCONHECIDO",
                        uid,
                        "NAO_AUTORIZADO"
                    )
                )

                con.commit()

    except KeyboardInterrupt:

        print("\nEncerrando...")
        break

    except Exception as e:

        print("ERRO:", e)

cur.close()
con.close()
arduino.close()
