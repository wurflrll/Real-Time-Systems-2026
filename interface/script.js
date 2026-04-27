const canvas = document.getElementById("render_canvas");
const ctx = canvas.getContext("2d");


// IO 
const submit_button = document.getElementById("submit_button");
submit_button.addEventListener("click", Start_Stream);



function Start_Stream() {
  let socket = new WebSocket ( "ws://187.124.174.169:8080/ws" );
  socket.binaryType = "blob";
  


  socket.addEventListener("open", () => {


    let arr = new Uint32Array([document.getElementById("second").value, 73, 1]);
    socket.send(arr.buffer);
    socket.send("Header end.");

  });

  socket.addEventListener("message", (e) => {
    //log(`RECEIVED: ${e.data}: ${counter}`);

    if (event.data instanceof Blob) {
    
      console.log("We have a blob!");
      const blob = new Blob([e.data], { type: "image/bmp" });
      
      console.log(blob.size);

      const img = new Image();
      img.onload = () => {
        // scale image to fit the existing canvas size
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        ctx.drawImage(
          img,
          0, 0, img.width, img.height,      // source image area
          0, 0, canvas.width, canvas.height // destination canvas area (scaled)
        );
      };
      img.src = URL.createObjectURL(blob);
    }
    else {
      console.log("not a blob");
    }
  });
}

// async function Start_Stream() {
//   const response = await fetch("http://187.124.174.169:8080/start?name=Jerry", {
//     method: "POST"
//   });
//   let string = await response.text();
//   console.log(string);
// }

// const totalFrames = 4;
// const fps = 1; 
// const frames = [];
// let currentFrame = 0;

// preload images
// for (let i = 1; i <= totalFrames; i++) {
//   const img = new Image();
//   img.src = `frames/frame_${String(i).padStart(4, '0')}.bmp`;
//   frames.push(img);
// }

// draw loop
// function draw() {
//   if (frames[currentFrame].complete) {
//     ctx.drawImage(frames[currentFrame], 0, 0, canvas.width, canvas.height);
//   }

//   currentFrame++;
//   if (currentFrame >= totalFrames) currentFrame = 0;

//   setTimeout(draw, 1000 / fps);
// }

// start after a short delay (to allow loading)