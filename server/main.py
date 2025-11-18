from fastapi import FastAPI
from fastapi.responses import HTMLResponse, FileResponse

server = FastAPI()


@server.get("/", response_class=HTMLResponse)
def root():
    with open("html/index.html", "r") as file:
        resp = file.read()
    return resp


@server.get("/leaderboard")
def get_leaderboard():
    return FileResponse("leaderboard.json")


if __name__ == "__main__":
    import uvicorn

    uvicorn.run(server)
