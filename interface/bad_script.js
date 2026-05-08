const canvas = document.getElementById("render_canvas");
const ctx = canvas.getContext("2d");


// IO 
const submit_button = document.getElementById("submit_button");
submit_button.addEventListener("click", Start_Stream);


let second_limit = 5000;

let frames_per_call = 20 * 24;


async function Start_Stream() {
  let second = parseInt(document.getElementById("start second").value);

  console.log("second: ", second);
  let total_frames = 0;

  while (second < second_limit) {
    console.log("read once");
    await ReadSet(second);
    //second += 20;
    //console.log("second: ", second);
    break;
    // frames_per_call / (24); // 24 fps is the standard frame rate
  }
}


async function ReadSet(second) { return new Promise( (resolve, reject) => {

  let counter = 0;
  let frames_received = 0;

  let socket = new WebSocket ( "ws://187.124.174.169:8080/ws" );
  socket.binaryType = "blob";

  socket.addEventListener("open", async () => {
    let arr = new Uint32Array([second, frames_per_call, 1]);
    socket.send(arr.buffer);
    //socket.send("Header end.");
  });



  let buffer;

  socket.addEventListener("message", async (event) => {
    //log(`RECEIVED: ${e.data}: ${counter}`)


    if (event.data instanceof Blob) {

      let image_buffer = new Uint8Array(await event.data.arrayBuffer());
      console.log("A BLOB!");
      console.log(image_buffer.slice(0, 20));

      const blob = new Blob([image_buffer], { type: "image/jpeg" });
      const img = new Image();
      img.onload = () => {
          // scale image to fit the existing canvas size
          ctx.clearRect(0, 0, canvas.width, canvas.height);
          // ctx.setTransform(1, 0, 0, -1, 0, canvas.height);
          ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
      };
      img.src = URL.createObjectURL(blob);


      // counter++;
      // if (counter == 1) {
      //   buffer = new Uint8Array(await event.data.arrayBuffer());
      // }
      // if (counter == 2) {
      //   let second_buffer = new Uint8Array(await event.data.arrayBuffer());
      //   let double_buffer = new Uint8Array(buffer.length + second_buffer.length);
      //   double_buffer.set(buffer);
      //   double_buffer.set(second_buffer, buffer.length);
      //   counter = 0;
      //   frames_received++;
      //   const decompressed = pako.inflate(double_buffer);
      //   const blob = new Blob([decompressed], { type: "image/jpg" });
      
      //   console.log(blob.size);

      //   const img = new Image();
      //   img.onload = () => {
      //     // scale image to fit the existing canvas size
      //     ctx.clearRect(0, 0, canvas.width, canvas.height);
      //     ctx.setTransform(1, 0, 0, -1, 0, canvas.height);
      //     ctx.drawImage(img, 0, 0, canvas.width, canvas.height);

      //     // ctx.drawImage(
      //     //   img,
      //     //   0, 0, img.width, img.height,      // source image area
      //     //   0, 0, canvas.width, canvas.height // destination canvas area (scaled)
      //     // );
      //     // 
      //     //document.getElementById("start second").value = second + frames_received / 24;
      //   };
      //   img.src = URL.createObjectURL(blob);
      //   if (frames_received == frames_per_call) {
      //     socket.close();
      //   }
      //}
    }
    else {
      console.log("not a blob");
      console.log(event.data);
    }
  });
  socket.addEventListener("error", (err) => {
    console.error("Socket error:", err);
    reject(err);
  });
  socket.addEventListener("close", (event) => {
    console.log("Socket closed:", event.code, event.reason);
    resolve();
  });
})}