var Socket;

var on = true;

var red;
var green;
var blue;

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':82/');
}

setInterval(function() {
  if(on){
    Socket.send("R"+red);
    Socket.send("G"+green);
    Socket.send("B"+blue);
  }
}, 500);

document.addEventListener('DOMContentLoaded', function () {
  var checkbox = document.querySelector('input[type="checkbox"]');

  checkbox.addEventListener('change', function () {
    if (checkbox.checked) {
      on = true;
    } else {
      Socket.send("R0");
      Socket.send("G0");
      Socket.send("B0");
      on = false;
    }
  });
});

var colorPicker = new iro.ColorPicker(".colorPicker", {
  // color picker options
  // Option guide: https://iro.js.org/guide.html#color-picker-options
  width: 350,
  color: "rgb(255, 0, 0)",
  borderWidth: 1,
  borderColor: "#fff",
});

 colorPicker.on(["color:init", "color:change"], function(color){
    red = ((color.rgbString).split(',')[0]).substring(4, 7);
    green = (color.rgbString).split(',')[1];
        // if((color.rgbString).split(',')[2]) ==
    var longueur =  ((color.rgbString).split(',')[2]).length;
    blue = (((color.rgbString).split(',')[2]).substring(0,longueur-1));


});