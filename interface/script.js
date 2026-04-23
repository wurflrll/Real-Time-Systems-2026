const canvas = document.getElementById("render_canvas");
const ctx = canvas.getContext("2d");


// IO 
const submit_button = document.getElementById("submit_button");
submit_button.addEventListener("click", Start_Stream);


let socket = new WebSocket ( "ws://187.124.174.169:8080/ws" );
  
socket.addEventListener("open", () => {


  socket.send("Hello Server!");


  let arr = new Uint8Array(72, 73, 1);
  socket.send(arr.buffer);
  socket.send("Header end.");

});

socket.addEventListener("message", (e) => {
  log(`RECEIVED: ${e.data}: ${counter}`);


  const decompressed = pako.inflate(e.data);
  const blob = new Blob([decompressed], { type: "image/bmp" });
  const url = URL.createObjectURL(blob);
  const img = new Image();
  img.src = url;
  counter++;
});
  

async function Start_Stream() {
  const response = await fetch("http://187.124.174.169:8080/start?name=Jerry", {
    method: "POST"
  });
  let string = await response.text();
  console.log(string);
}

const totalFrames = 4;
const fps = 1; 
const frames = [];
let currentFrame = 0;

// preload images
for (let i = 1; i <= totalFrames; i++) {
  const img = new Image();
  img.src = `frames/frame_${String(i).padStart(4, '0')}.bmp`;
  frames.push(img);
}

// draw loop
function draw() {
  if (frames[currentFrame].complete) {
    ctx.drawImage(frames[currentFrame], 0, 0, canvas.width, canvas.height);
  }

  currentFrame++;
  if (currentFrame >= totalFrames) currentFrame = 0;

  setTimeout(draw, 1000 / fps);
}

// start after a short delay (to allow loading)
setTimeout(draw, 500);