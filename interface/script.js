const canvas = document.getElementById("render_canvas");
const ctx = canvas.getContext("2d");

// IO
const submit_button = document.getElementById("submit_button");
submit_button.addEventListener("click", Start_Stream);

let second_limit = 5000;

let granularity = 100

// 24 fps * 20 seconds
let frames_per_call = 20 * 24;

const FPS = 24;
const FRAME_DELAY = 1000 / FPS;

async function Start_Stream() {
  let second = parseInt(document.getElementById("start second").value);

  console.log("Starting at second:", second);

  while (second < second_limit) {

    // request and play one chunk
    await ReadSet(second);

    // advance by the duration we just played
    second += frames_per_call / FPS;

    console.log("next second:", second);
  }
}

async function ReadSet(second) {
  return new Promise((resolve, reject) => {

    let socket = new WebSocket("ws://187.124.174.169:8080/ws");
    socket.binaryType = "blob";

    let frameQueue = [];
    let playing = false;
    let socketClosed = false;

    let num_sent = 0;

    socket.addEventListener("open", () => {
      console.log("Socket opened");

      // request frames
      let arr = new Uint32Array([second, frames_per_call, granularity]);
      socket.send(arr.buffer);
    });

    socket.addEventListener("message", async (event) => {

      if (event.data instanceof Blob) {

        let image_buffer = new Uint8Array(await event.data.arrayBuffer());

        const blob = new Blob([image_buffer], { type: "image/jpeg" });

        const img = new Image();

        img.onload = () => {
          frameQueue.push(img);

          // start playback once first frame arrives
          if (!playing) {
            PlayFrames();
          }
        };

        img.src = URL.createObjectURL(blob);

      } else {
        console.log("not a blob");
      }
    });

    async function PlayFrames() {
      playing = true;

      while (true) {

        if (frameQueue.length > 0) {

          console.log("NUM SENT: ", num_sent);

          const img = frameQueue.shift();

          ctx.clearRect(0, 0, canvas.width, canvas.height);
          ctx.drawImage(img, 0, 0, canvas.width, canvas.height);

          num_sent += 1;

          // maintain 24 FPS
          await new Promise(r => setTimeout(r, FRAME_DELAY));

        } else {

          // if socket already closed and no frames remain, done
          if (socketClosed) {
            break;
          }

          // wait briefly for more frames
          await new Promise(r => setTimeout(r, 1));
        }
      }

      resolve();
    }

    socket.addEventListener("error", (err) => {
      console.error("Socket error:", err);
      reject(err);
    });

    socket.addEventListener("close", (event) => {
      console.log("Socket closed:", event.code, event.reason);
      socketClosed = true;
    });
  });
}