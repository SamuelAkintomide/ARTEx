
let scene, camera, rendered, cube;

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function init3D(){
  scene = new THREE.Scene();
  scene.background = new THREE.Color(0xffffff);

  camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);

  renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

  document.getElementById('3Dcube').appendChild(renderer.domElement);

  // Create a geometry
  const geometry = new THREE.BoxGeometry(5, 1, 4);

  // Materials of each face
  var cubeMaterials = [
    new THREE.MeshBasicMaterial({color:0x03045e}),
    new THREE.MeshBasicMaterial({color:0x023e8a}),
    new THREE.MeshBasicMaterial({color:0x0077b6}),
    new THREE.MeshBasicMaterial({color:0x03045e}),
    new THREE.MeshBasicMaterial({color:0x023e8a}),
    new THREE.MeshBasicMaterial({color:0x0077b6}),
  ];

  const material = new THREE.MeshFaceMaterial(cubeMaterials);

  cube = new THREE.Mesh(geometry, material);
  scene.add(cube);
  camera.position.z = 5;
  renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize(){
  camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

}

window.addEventListener('resize', onWindowResize, false);

// Create the 3D representation
init3D();

// Create events for the sensor readings
if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener('temperature_reading', function(e) {
    console.log("temperature_reading", e.data);
    document.getElementById("temp").innerHTML = e.data;
  }, false);
  
  source.addEventListener('pressure_reading', function(e) {
    console.log("pressure_reading", e.data);
    document.getElementById("pressure").innerHTML = e.data;
  }, false);

  source.addEventListener('altitude_reading', function(e) {
    console.log("altitude_reading", e.data);
    document.getElementById("altitude").innerHTML = e.data;
  }, false);

  source.addEventListener('accelerometer_readings', function(e) {
    console.log("accelerometer_readings", e.data);
    var obj = JSON.parse(e.data);
    document.getElementById("accX").innerHTML = obj.accX;
    document.getElementById("accY").innerHTML = obj.accY;
    document.getElementById("accZ").innerHTML = obj.accZ;
  }, false);
}

function resetPosition(element){
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+element.id, true);
  console.log(element.id);
  xhr.send();
}

function updateCameraFeed() {
    fetch('/camera-feed') // Fetch the camera feed from the server
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.blob(); // Get the response as a blob
        })
        .then(blob => {
            // Convert the blob to a data URL
            const reader = new FileReader();
            reader.onload = () => {
                const dataUrl = reader.result;
                // Update the src attribute of the image element with the data URL
                document.getElementById('cameraFeed').src = dataUrl;
            };
            reader.readAsDataURL(blob);
        })
        .catch(error => console.error('There was a problem with fetching the camera feed:', error));
}

function turnSkirtOn() {
  fetch('/turnSkirtOn') // Send a GET request to turn the skirt on
    .then(response => {
      if (!response.ok) {
        throw new Error('Network response was not ok');
      }
      console.log('Skirt turned on successfully');
    })
    .catch(error => console.error('There was a problem turning the skirt on:', error));
}

function turnSkirtOff() {
  fetch('/turnSkirtOff') // Send a GET request to turn the skirt off
    .then(response => {
      if (!response.ok) {
        throw new Error('Network response was not ok');
      }
      console.log('Skirt turned off successfully');
    })
    .catch(error => console.error('There was a problem turning the skirt off:', error));
}

// Call updateCameraFeed() periodically to update the camera feed
setInterval(updateCameraFeed, 1000); // Adjust the interval as needed

