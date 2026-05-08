const canvas = document.getElementById("render_canvas");
const ctx = canvas.getContext("2d");

// IO
const submit_button = document.getElementById("submit_button");
submit_button.addEventListener("click", Start_Stream);

let second_limit = 5000;

const FPS = 24;
const FRAME_TIME = 1000 / FPS;

let frames_per_call = 20 * FPS;

async function Start_Stream() {
  let second = parseInt(document.getElementById("start second").value);

  console.log("second:", second);

  while (second < second_limit) {
    console.log("read once");

    await ReadSet(second);

    break;
  }
}


async function ReadSet(second) {
  return new Promise((resolve, reject) => {

    let socket = new WebSocket("ws://187.124.174.169:8080/ws");
    socket.binaryType = "blob";

    // Queue of decoded images waiting to display
    const frameQueue = [];

    let playbackStarted = false;

    let count = 0;

    let receive_count = 0;

    // ---------------------------------------------------
    // Fixed framerate renderer
    // ---------------------------------------------------
    function startPlaybackLoop() {
      if (playbackStarted) return;

      playbackStarted = true;

      const interval = setInterval(() => {

        // No frames available yet
        if (frameQueue.length === 0) {
          return;
        }

        count += 1;
        console.log("num frames: ", count);
    
        const img = frameQueue.shift();

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        ctx.drawImage(
          img,
          0,
          0,
          canvas.width,
          canvas.height
        );
        if (count == 480) {
          resolve();
        }

      }, FRAME_TIME);

      socket.addEventListener("close", () => {
        clearInterval(interval);
      });
    }

    socket.addEventListener("open", async () => {

      let arr = new Uint32Array([
        second,
        frames_per_call,
        1
      ]);

      socket.send(arr.buffer);
    });

    socket.addEventListener("message", async (event) => {

      if (!(event.data instanceof Blob)) {
        console.log("not a blob");
        console.log(event.data);
        return;
      }

      try {

        // Convert blob -> image
        const imageBitmap = await createImageBitmap(event.data);

        // Queue frame instead of rendering immediately
        receive_count += 1;
        frameQueue.push(imageBitmap);

        console.log("Received: ", receive_count);

        // Start playback once first frame arrives
        startPlaybackLoop();

      } catch (err) {
        console.error("Failed to decode frame:", err);
      }
    });

    socket.addEventListener("error", (err) => {
      console.error("Socket error:", err);
      reject(err);
    });

    socket.addEventListener("close", (event) => {
      console.log("Socket closed:", event.code, event.reason);
      //resolve();
    });
  });
}