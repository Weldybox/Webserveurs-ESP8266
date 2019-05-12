var Socket;

var on = true;

var red;
var green;
var blue;

function init() {
  Socket = new WebSocket('ws://' + window.location.hostname + ':82/');
}

window.onload=function(){

  var bouton = document.getElementById('btnMenu');
  var nav = document.getElementById('nav');
  bouton.onclick = function(e){
      if(nav.style.display=="block"){
          nav.style.display="none";
      }else{
          nav.style.display="block";
      }
  };
};

var position = [];
function chooseSave(){
  Papa.parse('save.csv', {
    header: false,
    download: true,
    dynamicTyping: true,
    complete: function(results) {
      console.log(results);
      for(var i=(((results.data[1]).length)-2);i>=0;i-=1){
        console.log(((results.data[1]).length));
        if(i > (((results.data[1]).length)-4)){
          var id= 'carre' + String((i%4)+1);
          var backgroundcolor = "rgb(" + (results.data[1])[i] + ")";
          position[parseInt((id.substr(5,6)),10)] = i;
          document.getElementById(id).style.background = backgroundcolor;
        }
      }
    }
  });
}

function setSaveColor(id){
  Papa.parse('save.csv', {
    header: false,
    download: true,
    dynamicTyping: true,
    complete: function(results) {
      console.log(position);
      var tableau = results.data[1];
      console.log(tableau[parseInt((id.substr(5,6)),10)-1]);
      colorPicker.color.rgbString = "rgb("+ tableau[position[parseInt((id.substr(5,6)),10)]] +")";
      console.log(index);
    }
  });
}

function saving(){

  Socket.send("sR"+red);
  Socket.send("sG"+green);
  Socket.send("sB"+blue);

}
setInterval(function() {
  if(on){
    /*var sauv = document.getElementById("sauv");
    sauv.style.backgroundColor = "rgb("+ red + "," + green + "," + blue + ")";*/

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
  //color: "rgb(255, 0, 0)",
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