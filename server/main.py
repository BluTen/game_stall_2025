import threading
import time
import json
import requests
import dotenv

import serial
from flask import Flask, current_app

""" Disable Flask's default logging to keep the output clean """
import logging  # noqa: E402

log = logging.getLogger("werkzeug")
log.setLevel(logging.ERROR)


app = Flask(__name__)
arduino = serial.Serial(port="COM5", baudrate=9600, timeout=None)
SUPABASE_URL = "https://onficegxsclrxpaejfzx.supabase.co/rest/v1/"
supabase_db = requests.Session()
supabase_db.headers.update(
    {
        "apikey": dotenv.get_key(".env", "SUPABASE_SERVICE_ROLE_KEY"),
    }
)

website_queue = []


@app.route("/")
def home():
    return current_app.send_static_file("index.html")


@app.route("/events", methods=["GET"])
def events():
    def event_stream():
        global website_queue
        while True:
            if website_queue:
                event = website_queue.pop(0)
                yield f"data: {event[0]}\n{json.dumps(event[1])}\n\n"
            else:
                time.sleep(0.1)

    return app.response_class(event_stream(), mimetype="text/event-stream")


def cmd_interface():
    # Wait for server to start
    global website_queue

    time.sleep(2)
    while True:
        while not arduino.is_open:
            input("Arduino not connected. Press ENTER to retry...")
            try:
                arduino.open()
            except serial.SerialException:
                continue

        name = input("Enter name: ")
        class_ = input("Enter class: ")

        arduino.write(b"S")
        website_queue.append(
            (
                "start-game",
                {
                    "name": name,
                    "class": class_,
                },
            )
        )
        # TODO: Add game start music

        while True:
            duration = arduino.readline()
            while duration[0] == ord("#"):
                print("arduino:", duration[1:].strip().decode())
                duration = arduino.readline()

            if duration[0] != ord("D"):
                print(
                    f"Got '{duration.decode()}'. Desync between arduino and computer. Resetting, reset arduino manually..."
                )
                break
            try:
                duration = float(duration[1:].strip())
            except ValueError:
                print("Unable to get duration. Resetting, reset arduino manually...")
                break
            # TODO: add gameplay music
            next_event = arduino.read()[0]
            while next_event == ord("#"):
                print("arduino:", arduino.readline().strip().decode())
                next_event = arduino.read()[0]
            if next_event == ord("H"):
                # TODO: add hit sound sfx
                print("Hit!")
                website_queue.append(
                    (
                        "increment-score",
                        {},
                    )
                )
                continue
            elif next_event == ord("R"):
                print("Missed but life left restarting...")
                website_queue.append(
                    (
                        "reset-score",
                        {},
                    )
                )
                continue
            elif next_event == ord("X"):
                # TODO: Game over sfx
                print("Game over!")
                score = arduino.readline().strip()
                try:
                    score = int(score)
                except ValueError:
                    print(f"Got: '{score.decode()}'")
                    print("Unable to get score. Resetting, reset arduino manually...")
                    break
                print(f"Score: {score}")
                print("Checking database for name...")
                # Store data to be sent to website
                supabase_db.post(
                    SUPABASE_URL + "leaderboard",
                    json={
                        "name": name,
                        "class": class_,
                        "score": score,
                    },
                )
                # Send ping to local page to update its display
                website_queue.append(
                    (
                        "end-score",
                        {
                            "name": name,
                            "class": class_,
                            "score": score,
                        },
                    )
                )
                break

        time.sleep(1)


if __name__ == "__main__":
    threading.Thread(target=cmd_interface, daemon=True).start()
    app.run()
